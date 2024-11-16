#include "StepSourceClass.hpp"
#include "toolgui/NodeMacros.h"
#include "optimizer/NLEquation.hpp"
#include "optimizer/DiffEquation.hpp"
#include "optimizer/NLStatefulEquation.hpp"
#include "optimizer/Observer.hpp"
#include "optimizer/SourceEq.hpp"

#include <numbers>
#include <cmath>

static const std::vector<node::model::BlockProperty> ClassProperties{
	node::model::BlockProperty{"Initial Value", node::model::BlockPropertyType::FloatNumber, 0.0 },
	node::model::BlockProperty{"Final Value", node::model::BlockPropertyType::FloatNumber, 1.0 },
	node::model::BlockProperty{"Step Time", node::model::BlockPropertyType::FloatNumber, 1.0 },
	node::model::BlockProperty{"Rise Time", node::model::BlockPropertyType::FloatNumber, 1e-6 },
};
static constexpr std::string_view Description = "Output = t < Step Time ? Initial Value : Final Value";

node::StepSourceClass::StepSourceClass()
	:BlockClass("Step")
{
}

const std::vector<node::model::BlockProperty>& node::StepSourceClass::GetClassProperties()
{
	return ClassProperties;
}

std::vector<node::model::BlockSocketModel::SocketType> node::StepSourceClass::CalculateSockets(const std::vector<model::BlockProperty>& properties)
{
	UNUSED_PARAM(properties);
	assert(ValidateClassProperties(properties));
	return {
		node::model::BlockSocketModel::SocketType::output
	};
}

const std::string_view& node::StepSourceClass::GetDescription() const
{
	return Description;
}

bool node::StepSourceClass::ValidateClassProperties(const std::vector<model::BlockProperty>& properties)
{
	if (properties.size() != ClassProperties.size())
	{
		return false;
	}
	for (size_t i = 0; i < properties.size(); i++)
	{
		if (properties[i].name != ClassProperties[i].name)
		{
			return false;
		}
		if (properties[i].type != ClassProperties[i].type)
		{
			return false;
		}
		if (!std::holds_alternative<double>(properties[i].prop))
		{
			return false;
		}
	}
	return true;
}

node::BlockType node::StepSourceClass::GetBlockType(const std::vector<model::BlockProperty>& properties)
{
	UNUSED_PARAM(properties);
	return BlockType::Source;
}

node::BlockClass::GetFunctorResult node::StepSourceClass::GetFunctor(const std::vector<model::BlockProperty>& properties)
{
	assert(ValidateClassProperties(properties));
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

	auto eq = opt::SourceEq{
		{0},
		[initial_value, final_value, event_set](std::span<double> out, const double& t, opt::SourceEq&) mutable
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
			
		},
		{
		[event_set, triggered = false](const double&, opt::SourceEq& eq) mutable
		{
			assert(eq.GetEvent());
			auto& event = *eq.GetEvent();
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
		}
	};
	eq.GetEvent() = opt::SourceEq::SourceEvent{step_time, false};
	return eq;
}

