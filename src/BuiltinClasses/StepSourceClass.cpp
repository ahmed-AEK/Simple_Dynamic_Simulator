#include "StepSourceClass.hpp"

static const std::vector<node::model::BlockProperty> ClassProperties{
	*node::model::BlockProperty::Create("Initial Value", node::model::BlockPropertyType::FloatNumber, 0.0 ),
	*node::model::BlockProperty::Create("Final Value", node::model::BlockPropertyType::FloatNumber, 1.0 ),
	*node::model::BlockProperty::Create("Step Time", node::model::BlockPropertyType::FloatNumber, 1.0 ),
	*node::model::BlockProperty::Create("Rise Time", node::model::BlockPropertyType::FloatNumber, 1e-6 ),
};
static constexpr std::string_view Description = "Output = t < Step Time ? Initial Value : Final Value";
static constexpr node::model::BlockSocketModel::SocketType class_sockets[] = {
		node::model::BlockSocketModel::SocketType::output
};

node::StepSourceClass::StepSourceClass()
	:BuiltinBasicClass{ "Step",  ClassProperties, class_sockets, Description, BlockType::Source }
{
}

node::BlockClass::GetFunctorResult node::StepSourceClass::GetFunctor(const std::vector<model::BlockProperty>& properties) const
{
	[[maybe_unused]] LightValidatePropertiesNotifier notifier;
	auto valid = ValidateClassProperties(properties, notifier);
	if (notifier.errored || !valid)
	{
		return std::string{ "failed to validate properties" };
	}
	double initial_value = std::get<double>(properties[0].prop);
	double final_value = std::get<double>(properties[1].prop);
	double step_time = std::get<double>(properties[2].prop);
	double rise_time = std::get<double>(properties[3].prop);
	struct StepState
	{
		bool first_triggered = false;
		bool second_triggered = false;
		double first_trigger_time;
		double second_trigger_time;
		double third_trigger_time;
	};

	if (rise_time < 1e-9)
	{
		rise_time = 1e-9;
	}

	std::shared_ptr<StepState> event_set = std::make_unique<StepState>(
		StepState{
			false,
			false,
			step_time,
			step_time + rise_time,
			step_time + rise_time * 2,
		}
	);

	auto eq = opt::SourceEqWrapper{
		{0},
		opt::make_SourceEqn<opt::FunctorSourceEq>(opt::FunctorSourceEq::SourceFunctor{[initial_value, final_value, event_set](std::span<double> out, const double& t, opt::SourceEvent&) mutable
		{
			if (!event_set->first_triggered)
			{
				out[0] = initial_value;
				return;
			}
			if (event_set->second_triggered)
			{
				out[0] = final_value;
				return;
			}

			out[0] = initial_value + (t - event_set->first_trigger_time) / (event_set->second_trigger_time - event_set->first_trigger_time) * (final_value - initial_value);

		}
		},
		opt::FunctorSourceEq::SourceTrigger{
		[event_set, triggered = false](const double&, opt::SourceEvent& ev) mutable
		{
			auto& event = ev;
			if (!event_set->first_triggered)
			{
				event_set->first_triggered = true;
				event.t = event_set->second_trigger_time;
				event.set = false;
				return;
			}
			if (!event_set->second_triggered)
			{
				event_set->second_triggered = true;
				event.t = event_set->third_trigger_time;
				event.set = false;
				return;
			}
		}
		}),
		{}
	};
	eq.ev = opt::SourceEvent{true, false, step_time};
	return eq;
}

