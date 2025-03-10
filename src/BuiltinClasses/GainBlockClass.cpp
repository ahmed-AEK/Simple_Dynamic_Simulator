#include "GainBlockClass.hpp"

static const std::vector<node::model::BlockProperty> ClassProperties{
	*node::model::BlockProperty::Create("Multiplier", node::model::BlockPropertyType::FloatNumber, 1)
};

static constexpr std::string_view Description = "Sets its output to its input multiplied by \"Multiplier\"";


node::GainBlockClass::GainBlockClass()
	:BlockClass("Gain")
{
}

const std::vector<node::model::BlockProperty>& node::GainBlockClass::GetDefaultClassProperties()
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
	if (properties[0].GetType() != ClassProperties[0].GetType())
	{
		return false;
	}
	if (!std::holds_alternative<double>(properties[0].prop))
	{
		return false;
	}
	return true;
}

const std::string_view& node::GainBlockClass::GetDescription() const
{
	return Description;
}

node::BlockType node::GainBlockClass::GetBlockType(const std::vector<model::BlockProperty>& properties)
{
	UNUSED_PARAM(properties);
	return BlockType::Stateless;
}

node::BlockClass::GetFunctorResult node::GainBlockClass::GetFunctor(const std::vector<model::BlockProperty>& properties)
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

