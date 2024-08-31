#include "ConstantSourceClass.hpp"
#include "toolgui/NodeMacros.h"
#include "optimizer/NLEquation.hpp"
#include "optimizer/DiffEquation.hpp"
#include "optimizer/NLStatefulEquation.hpp"
#include "optimizer/Observer.hpp"
#include "optimizer/SourceEq.hpp"

static const std::vector<node::model::BlockProperty> ClassProperties{
	node::model::BlockProperty{"Value", node::model::BlockPropertyType::FloatNumber, 1}
};
static constexpr std::string_view Description = "Output = Value";

node::ConstantSourceClass::ConstantSourceClass()
	:BlockClass("Constant Source")
{
}

const std::vector<node::model::BlockProperty>& node::ConstantSourceClass::GetClassProperties()
{
	return ClassProperties;
}

std::vector<node::model::BlockSocketModel::SocketType> node::ConstantSourceClass::CalculateSockets(const std::vector<model::BlockProperty>& properties)
{
	UNUSED_PARAM(properties);
	assert(ValidateClassProperties(properties));
	return {
		node::model::BlockSocketModel::SocketType::output
	};
}

const std::string_view& node::ConstantSourceClass::GetDescription() const
{
	return Description;
}

bool node::ConstantSourceClass::ValidateClassProperties(const std::vector<model::BlockProperty>& properties)
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

node::BlockType node::ConstantSourceClass::GetBlockType(const std::vector<model::BlockProperty>& properties)
{
	UNUSED_PARAM(properties);
	return BlockType::Source;
}

node::BlockFunctor node::ConstantSourceClass::GetFunctor(const std::vector<model::BlockProperty>& properties)
{
	assert(properties.size() == 1);
	assert(std::holds_alternative<double>(properties[0].prop));
	double value = std::get<double>(properties[0].prop);
	return opt::SourceEq{
		{0},
		[value](std::span<double> out, const double&)
		{
			out[0] = value;
		}
	};
}

