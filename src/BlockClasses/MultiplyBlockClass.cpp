#include "MultiplyBlockClass.hpp"
#include "optimizer/NLEquation.hpp"
#include "optimizer/DiffEquation.hpp"
#include "optimizer/NLStatefulEquation.hpp"
#include "optimizer/Observer.hpp"
#include "optimizer/SourceEq.hpp"

static const std::vector<node::model::BlockProperty> ClassProperties{
};

static constexpr std::string_view Description = "output = input1 * input2";


node::MultiplyBlockClass::MultiplyBlockClass()
	:BlockClass("Multiply")
{
}

const std::vector<node::model::BlockProperty>& node::MultiplyBlockClass::GetClassProperties()
{
	return ClassProperties;
}

std::vector<node::model::BlockSocketModel::SocketType>
node::MultiplyBlockClass::CalculateSockets(const std::vector<model::BlockProperty>& properties)
{
	UNUSED_PARAM(properties);
	assert(ValidateClassProperties(properties));
	return {
		node::model::BlockSocketModel::SocketType::input, 		
		node::model::BlockSocketModel::SocketType::input,
		node::model::BlockSocketModel::SocketType::output 
	};
}

bool node::MultiplyBlockClass::ValidateClassProperties(const std::vector<model::BlockProperty>& properties)
{
	if (properties.size() != 0)
	{
		return false;
	}
	return true;
}

const std::string_view& node::MultiplyBlockClass::GetDescription() const
{
	return Description;
}

node::BlockType node::MultiplyBlockClass::GetBlockType(const std::vector<model::BlockProperty>& properties)
{
	UNUSED_PARAM(properties);
	return BlockType::Stateless;
}

node::BlockFunctor node::MultiplyBlockClass::GetFunctor(const std::vector<model::BlockProperty>& properties)
{
	assert(properties.size() == 0);
	UNUSED_PARAM(properties);
	return opt::NLEquation{
		{0,1},
		{2},
		[](std::span<const double> in, std::span<double> out)
		{
			out[0] = in[0] * in[1];
		}
	};
}

