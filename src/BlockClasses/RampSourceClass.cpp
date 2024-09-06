#include "RampSourceClass.hpp"
#include "toolgui/NodeMacros.h"
#include "optimizer/NLEquation.hpp"
#include "optimizer/DiffEquation.hpp"
#include "optimizer/NLStatefulEquation.hpp"
#include "optimizer/Observer.hpp"
#include "optimizer/SourceEq.hpp"

static const std::vector<node::model::BlockProperty> ClassProperties{
	node::model::BlockProperty{"Slope", node::model::BlockPropertyType::FloatNumber, 1.0 }
};
static constexpr std::string_view Description = "Output = Slope * t";

node::RampSourceClass::RampSourceClass()
	:BlockClass("Ramp")
{
}

const std::vector<node::model::BlockProperty>& node::RampSourceClass::GetClassProperties()
{
	return ClassProperties;
}

std::vector<node::model::BlockSocketModel::SocketType> node::RampSourceClass::CalculateSockets(const std::vector<model::BlockProperty>& properties)
{
	UNUSED_PARAM(properties);
	assert(ValidateClassProperties(properties));
	return {
		node::model::BlockSocketModel::SocketType::output
	};
}

const std::string_view& node::RampSourceClass::GetDescription() const
{
	return Description;
}

bool node::RampSourceClass::ValidateClassProperties(const std::vector<model::BlockProperty>& properties)
{
	if (properties.size() != 1)
	{
		return false;
	}
	if (properties[0].name != ClassProperties[0].name)
	{
		return false;
	}
	if (properties[0].type != ClassProperties[0].type)
	{
		return false;
	}
	if (!std::holds_alternative<double>(properties[0].prop))
	{
		return false;
	}
	return true;
}

node::BlockType node::RampSourceClass::GetBlockType(const std::vector<model::BlockProperty>& properties)
{
	UNUSED_PARAM(properties);
	return BlockType::Source;
}

node::BlockFunctor node::RampSourceClass::GetFunctor(const std::vector<model::BlockProperty>& properties)
{
	assert(properties.size() == 1);
	assert(std::holds_alternative<double>(properties[0].prop));
	double value = std::get<double>(properties[0].prop);
	return opt::SourceEq{
		{0},
		[value](std::span<double> out, const double& t)
		{
			out[0] = value * t;
		}
	};
}

