#include "GainBlockClass.hpp"
#include "optimizer/NLEquation.hpp"
#include "optimizer/DiffEquation.hpp"
#include "optimizer/NLStatefulEquation.hpp"
#include "optimizer/Observer.hpp"

static const std::vector<node::model::BlockProperty> ClassProperties{
	node::model::BlockProperty{"Multiplier", node::model::BlockPropertyType::FloatNumber, 1}
};

node::GainBlockClass::GainBlockClass()
	:BlockClass("Gain")
{
}

const std::vector<node::model::BlockProperty>& node::GainBlockClass::GetClassProperties()
{
	return ClassProperties;
}

std::vector<node::model::BlockSocketModel::SocketType>
node::GainBlockClass::CalculateSockets(const std::vector<model::BlockProperty>& properties)
{
	UNUSED_PARAM(properties);
	assert(ValidateClassProperties(properties));
	return {
		node::model::BlockSocketModel::SocketType::input, 
		node::model::BlockSocketModel::SocketType::output 
	};
}

bool node::GainBlockClass::ValidateClassProperties(const std::vector<model::BlockProperty>& properties)
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

node::BlockType node::GainBlockClass::GetBlockType(const std::vector<model::BlockProperty>& properties)
{
	UNUSED_PARAM(properties);
	return BlockType::Stateless;
}

node::BlockFunctor node::GainBlockClass::GetFunctor(const std::vector<model::BlockProperty>& properties)
{
	assert(properties.size() == 1);
	assert(std::holds_alternative<double>(properties[0].prop));
	double multiplier = std::get<double>(properties[0].prop);
	return opt::NLEquation{
		{0},
		{1},
		[multiplier](std::span<const double> in, std::span<double> out)
		{
			out[0] = in[0] * multiplier;
		}
	};
}

