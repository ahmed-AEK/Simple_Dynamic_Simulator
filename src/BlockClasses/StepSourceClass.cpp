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

	auto eq = opt::SourceEq{
		{0},
		[initial_value, final_value, event_set = false](std::span<double> out, const double&, opt::SourceEq& eq) mutable
		{
			if (!event_set)
			{
				if (eq.GetEvent() && eq.GetEvent()->set)
				{
					event_set = true;
				}
			}
			if (event_set)
			{
				out[0] = final_value;
			}
			else
			{
				out[0] = initial_value;
			}
			
		}
	};
	eq.GetEvent() = opt::SourceEq::SourceEvent{step_time, false};
	return std::move(eq);
}

