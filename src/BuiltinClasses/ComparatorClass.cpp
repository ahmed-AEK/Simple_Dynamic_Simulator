#include "ComparatorClass.hpp"

static const std::vector<node::model::BlockProperty> ClassProperties{
	*node::model::BlockProperty::Create("threshold", node::model::BlockPropertyType::FloatNumber, 0.0 ),
	*node::model::BlockProperty::Create("Rise Time", node::model::BlockPropertyType::FloatNumber, 1e-6 ),
};

static constexpr std::string_view Description = "Output = input > threshold ? 1 : 0";
static constexpr node::model::SocketType class_sockets[] = {
		node::model::BlockSocketModel::SocketType::input,
		node::model::BlockSocketModel::SocketType::output
};

static constexpr double high_value = 1;
static constexpr double low_value = 0;

node::ComparatorBlockClass::ComparatorBlockClass()
	:BuiltinBasicClass{"Comparator", ClassProperties, class_sockets, Description, BlockType::Stateful}
{
}

int node::ComparatorBlockClass::GetFunctor(std::span<const model::BlockProperty> properties, IGetFunctorCallback& cb) const
{

	struct ComparatorOutputTransition
	{
		double trigger_time;
		double start_value;
		double end_time;
		bool rising;
	};

	[[maybe_unused]] LightValidatePropertiesNotifier notifier;
	auto valid = ValidateClassProperties(properties, notifier);
	if (!valid || notifier.errored)
	{
		cb.error("failed to validate properties");
		return false;
	}

	double threshold = std::get<double>(properties[0].prop);
	double rise_time = std::get<double>(properties[1].prop);
	if (rise_time < 1e-9)
	{
		rise_time = 1e-9;
	}
	constexpr auto transition = ComparatorOutputTransition{0, 0, 0, false};

	class ComparatorClassFunction: public opt::INLStatefulEquation
	{
	public:
		ComparatorClassFunction(ComparatorOutputTransition transitions, double rise_time, double threshold)
			: m_transition_times{std::move(transitions)}, m_rise_time{rise_time}, m_threshold{threshold}
		{
		}
		opt::Status Apply(std::span<const double> input, std::span<double> output, double t, opt::NLStatefulEquationDataCRef data) override
		{
			UNUSED_PARAM(input);
			const auto& crossings = data.crossings;
			assert(crossings.size());

			double end_value;
			if (crossings[0].current_value == opt::ZeroCrossDescriptor::Position::above)
			{
				end_value = high_value;
			}
			else
			{
				end_value = low_value;
			}

			if (t >= m_transition_times.end_time)
			{
				output[0] = end_value;
			}
			else
			{
				double current_value = m_transition_times.start_value + 
					(end_value - m_transition_times.start_value) * (t - m_transition_times.trigger_time) / m_rise_time;
				output[0] = current_value;
			}
			return opt::Status::ok;
		}
		opt::Status Update(std::span<const double> input, double t, opt::NLStatefulEquationDataRef data) override
		{
			UNUSED_PARAM(input);
			UNUSED_PARAM(t);
			UNUSED_PARAM(data);
			return opt::Status::ok;
		}
		opt::Status CrossTrigger(double t, size_t index, opt::NLStatefulEquationDataRef data) override 
		{
			UNUSED_PARAM(index);
			assert(index == 0);
			const auto& crossings = data.crossings;

			double start_value = 0;
			if (crossings[0].last_value == opt::ZeroCrossDescriptor::Position::above)
			{
				start_value = high_value;
			}
			else
			{
				start_value = low_value;
			}

			if (t >= m_transition_times.end_time)
			{
				m_transition_times.trigger_time = t;
				m_transition_times.start_value = start_value;
				m_transition_times.end_time = t + m_rise_time;
				m_transition_times.rising = crossings[0].current_value == opt::ZeroCrossDescriptor::Position::above;
				double end_value = m_transition_times.rising ? high_value : low_value;
				if (start_value != end_value)
				{
					// create transition
					data.ev.set = false;
					data.ev.t = m_transition_times.end_time;
				}
			}
			else
			{
				// transition in progress
				double old_end_value = start_value;
				start_value = m_transition_times.start_value + 
					(old_end_value - m_transition_times.start_value) * (t - m_transition_times.trigger_time) / m_rise_time;
				m_transition_times.start_value = start_value;
				m_transition_times.trigger_time = t;
				m_transition_times.rising = crossings[0].current_value == opt::ZeroCrossDescriptor::Position::above;
				double end_value = m_transition_times.rising ? high_value : low_value;
				m_transition_times.end_time = t + std::abs(end_value - m_transition_times.start_value) / (high_value - low_value) * m_rise_time;
				if (m_transition_times.end_time > t)
				{
					// push end_time away anway
					data.ev.set = false;
					data.ev.t = m_transition_times.end_time;
				}
			}
			return opt::Status::ok;
		}
	private:
		ComparatorOutputTransition m_transition_times;
		double m_rise_time;
		double m_threshold;
	};
	auto ret = opt::NLStatefulEquationWrapper{
		{0},
		{1},
		opt::make_NLStatefulEqn<ComparatorClassFunction>(transition, rise_time, threshold),
		{}
	};
	ret.data.crossings.push_back({
		threshold,
		0,
		opt::ZeroCrossDescriptor::CrossType::both,
		opt::ZeroCrossDescriptor::Position::undefined,
		opt::ZeroCrossDescriptor::Position::undefined,
		});
	ret.data.ev = opt::StatefulEquationEvent{true, true, 0};
	node::BlockView view{ ret };
	cb.call({ &view,1 });
	return true;
}



