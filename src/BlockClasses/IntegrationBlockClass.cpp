#include "IntegrationBlockClass.hpp"
#include "optimizer/NLEquation.hpp"
#include "optimizer/DiffEquation.hpp"
#include "optimizer/NLStatefulEquation.hpp"
#include "optimizer/Observer.hpp"
#include "optimizer/SourceEq.hpp"

static const std::vector<node::model::BlockProperty> ClassProperties{
};

static constexpr std::string_view Description = "output = integration(input)";


node::IntegrationBlockClass::IntegrationBlockClass()
	:BlockClass("Integration")
{
}

const std::vector<node::model::BlockProperty>& node::IntegrationBlockClass::GetClassProperties()
{
	return ClassProperties;
}

std::vector<node::model::BlockSocketModel::SocketType>
node::IntegrationBlockClass::CalculateSockets(const std::vector<model::BlockProperty>& properties)
{
	UNUSED_PARAM(properties);
	assert(ValidateClassProperties(properties));
	return {
		node::model::BlockSocketModel::SocketType::input, 
		node::model::BlockSocketModel::SocketType::output 
	};
}

bool node::IntegrationBlockClass::ValidateClassProperties(const std::vector<model::BlockProperty>& properties)
{
	if (properties.size() != 0)
	{
		return false;
	}
	return true;
}

const std::string_view& node::IntegrationBlockClass::GetDescription() const
{
	return Description;
}

node::BlockType node::IntegrationBlockClass::GetBlockType(const std::vector<model::BlockProperty>& properties)
{
	UNUSED_PARAM(properties);
	return BlockType::Differential;
}

node::BlockFunctor node::IntegrationBlockClass::GetFunctor(const std::vector<model::BlockProperty>& properties)
{
	assert(properties.size() == 0);
	UNUSED_PARAM(properties);
	return opt::DiffEquation{
		{0},
		{1},
		[](std::span<const double> in, std::span<double> out, const double)
		{
			out[0] = in[0];
		}
	};
}

