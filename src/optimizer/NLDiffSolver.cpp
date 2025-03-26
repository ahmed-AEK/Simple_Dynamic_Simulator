#include "NLDiffSolver.hpp"
#include "DiffEquation.hpp"
#include "NLEquation.hpp"
#include "SourceEq.hpp"
#include "Observer.hpp"

namespace opt
{
	struct ObserverSlot
	{
		size_t id;
		ObserverWrapper o;
	};
}

opt::NLDiffSolver::NLDiffSolver()
{
	auto preprocessor = [this](auto& state, const double time)
		{
			auto result = this->ApplySources(state, time);
			if (!result)
			{
				return result;
			}
			return m_NLSolver.Solve(state, time);
		};
	m_diffSolver.SetPreprocessor(std::move(preprocessor));

	auto postprocessor = [this](auto& state, const double time)
		{
			auto result = this->ApplySources(state, time);
			if (!result)
			{
				return result;
			}
			return m_NLSolver.Solve(state, time);
		};
	m_diffSolver.SetPostprocessor(std::move(postprocessor));
}

opt::NLDiffSolver::~NLDiffSolver()
{
}

void opt::NLDiffSolver::SetMaxStep(double step_size)
{
	m_diffSolver.SetMaxStep(step_size);
}

void opt::NLDiffSolver::AddDiffEquation(DiffEquationWrapper eq)
{
	m_diffSolver.AddEquation(std::move(eq));
}

void opt::NLDiffSolver::AddNLEquation(NLEquationWrapper eq)
{
	m_NLSolver.AddEquation(std::move(eq));
}

void opt::NLDiffSolver::AddNLStatefulEquation(NLStatefulEquationWrapper eq)
{
	m_NLSolver.AddStatefulEquation(std::move(eq));
}

void opt::NLDiffSolver::AddBufferEquation(BufferEquation eq)
{
	m_NLSolver.AddBufferEquation(std::move(eq));
}

size_t opt::NLDiffSolver::AddObserver(ObserverWrapper obs)
{
	size_t new_id = 0;
	if (m_observers.size())
	{
		new_id = m_observers.back().id + 1;
	}
	m_observers.emplace_back(new_id, std::move(obs));
	return new_id;
}

void opt::NLDiffSolver::AddSource(SourceEqWrapper source)
{
	m_sources.push_back(std::move(source));
}

opt::NLSolveResult opt::NLDiffSolver::Initialize(double start_time, double end_time)
{
	m_diffSolver.Initialize(start_time, end_time);
	m_NLSolver.Initialize();
	for (auto& observer : m_observers)
	{
		auto res = observer.o.equation->Initialize(start_time, end_time);
		if (res == Status::error)
		{ 
			return tl::unexpected<std::string>{observer.o.equation->GetLastError()};
		}
	}
	return std::monostate{};
}

opt::NLSolveResult opt::NLDiffSolver::CalculateInitialConditions(FlatMap& state)
{
	auto result = ApplySources(state, GetCurrentTime());
	if (!result)
	{
		return result;
	}
	result = m_NLSolver.Solve(state, GetCurrentTime());
	if (!result)
	{
		return result;
	}
	result = m_NLSolver.UpdateState(state, GetCurrentTime());
	if (!result)
	{
		return result;
	}
	result = NotifyObservers(state, GetCurrentTime());
	if (!result)
	{
		return result;
	}
	return NotifyZeroCrossings(state, GetCurrentTime());
}

opt::NLSolveResult opt::NLDiffSolver::NotifyObservers(const FlatMap& state, const double t)
{
	std::array<double, 20> input_tmp_buffer;
	for (auto& observer : m_observers)
	{
		const auto& input_ids = observer.o.input_ids;
		auto input_buffer = std::span{input_tmp_buffer}.subspan(0, input_ids.size());
		assert(input_buffer.size() == input_ids.size());
		for (size_t i = 0; i < input_buffer.size(); i++)
		{
			const auto val = state.get(input_ids[i]);
			input_buffer[i] = val;
		}
		auto res = observer.o.equation->Apply(input_buffer, t);
		if (res == Status::error)
		{
			return tl::unexpected<std::string>{observer.o.equation->GetLastError() };
		}
	}
	return std::monostate{};
}

std::vector<opt::ObserverData> opt::NLDiffSolver::GetObserversData()
{
	std::vector<ObserverData> result;
	for (auto&& [id, observer] : m_observers)
	{
		result.emplace_back(id, observer.equation->GetResults());
	}
	return result;
}

opt::StepResult opt::NLDiffSolver::Step(FlatMap& state)
{
	double start_time = GetCurrentTime();
	std::optional<double> next_event;
	UpdateSources(GetCurrentTime());
	if (m_source_event_times.size())
	{
		next_event = m_source_event_times[0];
	}
	
	m_diffSolver.SetNextEventTime(next_event);

	const auto result = m_diffSolver.Step(state);
	
	{
		auto post_result = m_diffSolver.ApplyPostProcessor(state, GetCurrentTime()); // update sources and NLEquations 
		if (!post_result)
		{
			return opt::StepResult::unexpected_type{ std::move(post_result.error()) };
		}
	}
	double end_time = GetCurrentTime();

	if (UpdateNewZeroCrossings(state))
	{
		double crossed_time = end_time;
		FlatMap::sync(state, m_crossed_states);
		FlatMap::sync(state, m_new_crossed_states);
		for (int i = 0; i < ZeroCrossingIterations; i++)
		{
			double current_time = (start_time + end_time) / 2;
			auto nl_result = InterpolateStateAt(m_new_crossed_states, current_time);
			if (!nl_result)
			{
				return tl::unexpected<std::string>{std::move(nl_result.error())};
			}
			bool crossing_contained = UpdateNewZeroCrossings(m_new_crossed_states);
			if (crossing_contained)
			{
				end_time = current_time;
				crossed_time = current_time;
				FlatMap::sync(m_new_crossed_states, m_crossed_states);
			}
			else
			{
				start_time = current_time;
			}
		}
		FlatMap::sync(m_crossed_states, state);
		{
			auto res = NotifyZeroCrossings(state, crossed_time);
			if (!res)
			{
				return tl::unexpected<std::string>{ std::move(res.error()) };
			}
		}
		m_diffSolver.SetCurrentTime(crossed_time);
	}

	if (next_event && GetCurrentTime() >= *next_event)
	{
		auto res = TriggerSources(GetCurrentTime());
		if (!res)
		{
			return tl::unexpected<std::string>{ std::move(res.error()) };
		}
	}
	{
		auto res = m_NLSolver.UpdateState(state, GetCurrentTime());
		if (!res)
		{
			return tl::unexpected<std::string>{ std::move(res.error()) };
		}
	}
	if (GetCurrentTime() - m_last_oberver_time >= 1e-9)
	{
		auto res = NotifyObservers(state, GetCurrentTime());
		if (!res)
		{
			return tl::unexpected<std::string>{ std::move(res.error()) };
		}
		m_last_oberver_time = GetCurrentTime();
	}

	if (result == opt::StepEnd::ReachedEnd)
	{
		for (auto& observer : m_observers)
		{
			auto res = observer.o.equation->CaptureEnd(GetCurrentTime());
			if (res == Status::error)
			{
				return tl::unexpected<std::string>{ observer.o.equation->GetLastError() };
			}
		}
	}
	return result;
}

opt::NLSolveResult opt::NLDiffSolver::ApplySources(FlatMap& state, const double t)
{
	std::array<double, 20> output_temp_buff;
	for (auto& source : m_sources)
	{
		const auto& output_ids = source.output_ids;
		auto output_buffer = std::span{output_temp_buff}.subspan(0, output_ids.size());
		auto apply_result = source.equation->Apply(output_buffer, t, source.ev);
		if (apply_result == Status::error)
		{
			return tl::unexpected{ source.equation->GetLastError() };
		}
		assert(output_buffer.size() == output_ids.size());
		for (size_t i = 0; i < output_buffer.size(); i++)
		{
			const double result = output_buffer[i];
			state.modify(output_ids[i], result);
		}
	}
	return std::monostate{};
}

opt::NLSolveResult opt::NLDiffSolver::TriggerSources(const double t)
{
	for (auto& source : m_sources)
	{
		auto&& event_ref = source.ev;
		if (!event_ref.enabled)
		{
			continue;
		}

		if (event_ref.t <= t && !event_ref.set)
		{
			event_ref.set = true;
			auto res = source.equation->EventTrigger(t, event_ref);
			if (res == Status::error)
			{
				return tl::unexpected(source.equation->GetLastError());
			}
		}

	}
	for (auto& eq : m_NLSolver.GetStatefulEquations())
	{
		auto&& event_ref = eq.data.ev;
		if (!event_ref.enabled)
		{
			continue;
		}

		if (event_ref.t <= t && !event_ref.set)
		{
			event_ref.set = true;
			auto res = eq.equation->EventTrigger(t, eq.data);
			if (res == Status::error)
			{
				return tl::unexpected{ eq.equation->GetLastError() };
			}
		}
	}
	return std::monostate{};
}

opt::NLSolveResult opt::NLDiffSolver::NotifyZeroCrossings(const FlatMap& state, const double t)
{
	for (auto&& eq : m_NLSolver.GetStatefulEquations())
	{
		auto& zero_crossings = eq.data.crossings;
		if (!zero_crossings.size())
		{
			continue;
		}
		int32_t idx = 0;
		for (auto&& crossing : zero_crossings)
		{
			assert(eq.input_ids.size() > static_cast<size_t>(crossing.in_port_id));
			const auto& port_id = eq.input_ids[crossing.in_port_id];
			auto&& signal_value = state.get(port_id);
			auto old_state = crossing.current_value;
			auto new_state = (signal_value - crossing.value > 0) ? ZeroCrossDescriptor::Position::above : ZeroCrossDescriptor::Position::below;
			if (old_state == new_state)
			{
				continue;
			}
			else if (crossing.type == ZeroCrossDescriptor::CrossType::both)
			{
				crossing.last_value = crossing.current_value;
				crossing.current_value = new_state;
				auto res = eq.equation->CrossTrigger(t, idx, eq.data);
				if (res == Status::error)
				{
					return tl::unexpected<std::string>{ eq.equation->GetLastError() };
				}
			}
			else if (crossing.type == ZeroCrossDescriptor::CrossType::falling && 
				new_state == ZeroCrossDescriptor::Position::below)
			{
				crossing.last_value = crossing.current_value;
				crossing.current_value = new_state;
				auto res = eq.equation->CrossTrigger(t, idx, eq.data);
				if (res == Status::error)
				{
					return tl::unexpected<std::string>{	eq.equation->GetLastError() };
				}
			}
			else if (crossing.type == ZeroCrossDescriptor::CrossType::rising && 
				new_state == ZeroCrossDescriptor::Position::above)
			{
				crossing.last_value = crossing.current_value;
				crossing.current_value = new_state;
				auto res = eq.equation->CrossTrigger(t, idx, eq.data);
				if (res == Status::error)
				{
					return tl::unexpected<std::string>{ eq.equation->GetLastError() };
				}
			}
			idx++;

		}
	}
	return std::monostate{};
}

bool opt::NLDiffSolver::UpdateNewZeroCrossings(const FlatMap& state)
{
	m_new_zero_crossed_blocks.clear();

	int32_t index = 0;
	for (auto&& eq : m_NLSolver.GetStatefulEquations())
	{
		const auto& zero_crossings = eq.data.crossings;
		if (!zero_crossings.size())
		{
			index++;
			continue;
		}
		for (auto&& crossing : zero_crossings)
		{
			assert(eq.input_ids.size() > static_cast<size_t>(crossing.in_port_id));
			const auto& port_id = eq.input_ids[crossing.in_port_id];
			auto&& signal_value = state.get(port_id);
			auto old_state = crossing.current_value;
			auto new_state = (signal_value - crossing.value > 0) ? ZeroCrossDescriptor::Position::above : ZeroCrossDescriptor::Position::below;
			if (old_state == new_state)
			{
				continue;
			}
			else if (crossing.type == ZeroCrossDescriptor::CrossType::both)
			{
				if (m_new_zero_crossed_blocks.size() == 0 || index == m_new_zero_crossed_blocks.back())
				{
					m_new_zero_crossed_blocks.push_back(index);
				}
			}
			else if (crossing.type == ZeroCrossDescriptor::CrossType::falling
				&& new_state == ZeroCrossDescriptor::Position::below)
			{
				if (m_new_zero_crossed_blocks.size() == 0 || index == m_new_zero_crossed_blocks.back())
				{
					m_new_zero_crossed_blocks.push_back(index);
				}
			}
			else if (crossing.type == ZeroCrossDescriptor::CrossType::rising
				&& new_state == ZeroCrossDescriptor::Position::above)
			{
				if (m_new_zero_crossed_blocks.size() == 0 || index == m_new_zero_crossed_blocks.back())
				{
					m_new_zero_crossed_blocks.push_back(index);
				}
			}
		}
		index++;
	}
	return m_new_zero_crossed_blocks.size(); 
}

opt::NLSolveResult opt::NLDiffSolver::InterpolateStateAt(FlatMap& state, const double t)
{
	auto res = ApplySources(state, t);
	if (!res)
	{
		return res;
	}
	m_diffSolver.InterpolateAt(state, t);
	return m_NLSolver.Solve(state, t);
}

void opt::NLDiffSolver::UpdateSources(const double t)
{
	m_source_event_times.clear();
	for (auto& source : m_sources)
	{
		auto&& event_ref = source.ev;

		if(event_ref.enabled && event_ref.t > t)
		{
			m_source_event_times.push_back(event_ref.t);
		}
	}
	for (auto& eq : m_NLSolver.GetStatefulEquations())
	{
		auto&& event_ref = eq.data.ev;
		if (event_ref.enabled && event_ref.t > t)
		{
			m_source_event_times.push_back(event_ref.t);
		}
	}
	std::sort(m_source_event_times.begin(), m_source_event_times.end());
}
