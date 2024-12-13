#include "SineSourceClass.hpp"
#include "toolgui/NodeMacros.h"
#include "optimizer/NLEquation.hpp"
#include "optimizer/DiffEquation.hpp"
#include "optimizer/NLStatefulEquation.hpp"
#include "optimizer/Observer.hpp"
#include "optimizer/SourceEq.hpp"

#include <numbers>
#include <cmath>

static const std::vector<node::model::BlockProperty> ClassProperties{
	node::model::BlockProperty{"Phase_deg", node::model::BlockPropertyType::FloatNumber, 0.0 },
	node::model::BlockProperty{"Freq_hz", node::model::BlockPropertyType::FloatNumber, 1.0 },
};
static constexpr std::string_view Description = "Output = Sin(2*pi*freq + phase*pi/180)";

node::SineSourceClass::SineSourceClass()
	:BlockClass("Sine")
{
}

const std::vector<node::model::BlockProperty>& node::SineSourceClass::GetDefaultClassProperties()
{
	return ClassProperties;
}

std::vector<node::model::BlockSocketModel::SocketType> node::SineSourceClass::CalculateSockets(const std::vector<model::BlockProperty>& properties)
{
	UNUSED_PARAM(properties);
	assert(ValidateClassProperties(properties));
	return {
		node::model::BlockSocketModel::SocketType::output
	};
}

const std::string_view& node::SineSourceClass::GetDescription() const
{
	return Description;
}

bool node::SineSourceClass::ValidateClassProperties(const std::vector<model::BlockProperty>& properties)
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

node::BlockType node::SineSourceClass::GetBlockType(const std::vector<model::BlockProperty>& properties)
{
	UNUSED_PARAM(properties);
	return BlockType::Source;
}

node::BlockClass::GetFunctorResult node::SineSourceClass::GetFunctor(const std::vector<model::BlockProperty>& properties)
{
	assert(ValidateClassProperties(properties));
	double phase_rad = std::get<double>(properties[0].prop) / 180.0 * std::numbers::pi;
	double freq_rad = std::get<double>(properties[1].prop) * 2 * std::numbers::pi;
	return opt::SourceEq{
		{0},
		[phase_rad, freq_rad](std::span<double> out, const double& t, opt::SourceEq&)
		{
			out[0] = std::sin(t* freq_rad + phase_rad);
		}
	};
}

