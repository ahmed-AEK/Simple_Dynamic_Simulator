#include "NLDiffSolver.hpp"
#include "DiffEquation.hpp"
#include "NLEquation.hpp"
#include "toolgui/NodeMacros.h"
#include "SourceEq.hpp"
#include "Observer.hpp"
#include <algorithm>

namespace opt
{
	struct ObserverSlot
	{
		size_t id;
		Observer o;
	};
}

opt::NLDiffSolver::NLDiffSolver()
{
	auto preprocessor = [this](auto& state, const double time)
		{
			this->ApplySources(state, time);
			m_NLSolver.Solve(state, time);
		};
	m_diffSolver.SetPreprocessor(std::move(preprocessor));

	auto postprocessor = [this](auto& state, const double time)
		{
			this->ApplySources(state, time);
			m_NLSolver.Solve(state, time);
			m_NLSolver.UpdateState(state, time);
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

void opt::NLDiffSolver::AddDiffEquation(DiffEquation eq)
{
	m_diffSolver.AddEquation(std::move(eq));
}

void opt::NLDiffSolver::AddNLEquation(NLEquation eq)
{
	m_NLSolver.AddEquation(std::move(eq));
}

void opt::NLDiffSolver::AddNLStatefulEquation(NLStatefulEquation eq)
{
	m_NLSolver.AddStatefulEquation(std::move(eq));
}

size_t opt::NLDiffSolver::AddObserver(Observer obs)
{
	size_t new_id = 0;
	if (m_observers.size())
	{
		new_id = m_observers.back().id + 1;
	}
	m_observers.emplace_back(new_id, std::move(obs));
	return new_id;
}

void opt::NLDiffSolver::AddSource(SourceEq source)
{
	m_sources.push_back(std::move(source));
}

void opt::NLDiffSolver::Initialize(double start_time, double end_time)
{
	m_diffSolver.Initialize(start_time, end_time);
	m_NLSolver.Initialize();
	for (auto& observer : m_observers)
	{
		observer.o.Initialize(start_time, end_time);
	}
}

void opt::NLDiffSolver::CalculateInitialConditions(FlatMap& state)
{
	ApplySources(state, GetCurrentTime());
	m_NLSolver.Solve(state, GetCurrentTime());
	m_NLSolver.UpdateState(state, GetCurrentTime());
	NotifyObservers(state, GetCurrentTime());
}

void opt::NLDiffSolver::NotifyObservers(const FlatMap& state, const double t)
{
	for (auto& observer : m_observers)
	{
		auto input_buffer = observer.o.get_input_buffer();
		auto input_ids = observer.o.get_input_ids();
		assert(input_buffer.size() == input_ids.size());
		for (size_t i = 0; i < input_buffer.size(); i++)
		{
			const auto val = state.get(input_ids[i]);
			input_buffer[i] = val;
		}
		observer.o.Apply(t);
	}
}

std::vector<opt::ObserverData> opt::NLDiffSolver::GetObserversData()
{
	std::vector<ObserverData> result;
	for (auto&& [id, observer] : m_observers)
	{
		result.emplace_back(id, observer.GetResults());
	}
	return result;
}

opt::StepResult opt::NLDiffSolver::Step(FlatMap& state)
{
	std::optional<double> next_event;
	UpdateSources(GetCurrentTime());
	if (m_source_event_times.size())
	{
		next_event = m_source_event_times[0];
	}
	
	m_diffSolver.SetNextEventTime(next_event);

	const auto result = m_diffSolver.Step(state);
	m_diffSolver.ApplyPostProcessor(state, GetCurrentTime()); // update sources and NLEquations 
	if (next_event && GetCurrentTime() <= *next_event)
	{
		// event happened, notify before and after the update
		NotifyObservers(state, GetCurrentTime());
		UpdateSources(GetCurrentTime());
		m_diffSolver.ApplyPostProcessor(state, GetCurrentTime()); // update sources and NLEquations 
		NotifyObservers(state, GetCurrentTime());
		m_last_oberver_time = GetCurrentTime();
	}
	else if (GetCurrentTime() - m_last_oberver_time > 1e-4)
	{
		NotifyObservers(state, GetCurrentTime());
		m_last_oberver_time = GetCurrentTime();
	}

	if (result == opt::StepResult::ReachedEnd)
	{
		for (auto& observer : m_observers)
		{
			observer.o.CaptureEnd(GetCurrentTime());
		}
	}
	return result;
}

void opt::NLDiffSolver::ApplySources(FlatMap& state, const double t)
{
	for (auto& source : m_sources)
	{
		auto output_buffer = source.get_output_buffer();
		auto output_ids = source.get_output_ids();
		source.Apply(t);
		assert(output_buffer.size() == output_ids.size());
		for (size_t i = 0; i < output_buffer.size(); i++)
		{
			const double result = output_buffer[i];
			state.modify(output_ids[i], result);
		}
	}
}

void opt::NLDiffSolver::UpdateSources(const double t)
{
	m_source_event_times.clear();
	for (auto& source : m_sources)
	{
		auto&& event_ref = source.GetEvent();
		if (!event_ref)
		{
			continue;
		}

		if(event_ref->t <= t)
		{
			event_ref->set = true;
		}
		else
		{
			m_source_event_times.push_back(event_ref->t);
		}
	}
	std::sort(m_source_event_times.begin(), m_source_event_times.end());
}
