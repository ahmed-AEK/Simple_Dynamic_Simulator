#include "DerivativeBlockClass.hpp"
#include "optimizer/NLEquation.hpp"
#include "optimizer/DiffEquation.hpp"
#include "optimizer/NLStatefulEquation.hpp"
#include "optimizer/Observer.hpp"
#include "optimizer/SourceEq.hpp"

static const std::vector<node::model::BlockProperty> ClassProperties{
};

static constexpr std::string_view Description = "output = d(input)/dt";


node::DerivativeBlockClass::DerivativeBlockClass()
	:BlockClass("Derivative")
{
}

const std::vector<node::model::BlockProperty>& node::DerivativeBlockClass::GetClassProperties()
{
	return ClassProperties;
}

std::vector<node::model::BlockSocketModel::SocketType>
node::DerivativeBlockClass::CalculateSockets(const std::vector<model::BlockProperty>& properties)
{
	UNUSED_PARAM(properties);
	assert(ValidateClassProperties(properties));
	return {
		node::model::BlockSocketModel::SocketType::input, 
		node::model::BlockSocketModel::SocketType::output 
	};
}

bool node::DerivativeBlockClass::ValidateClassProperties(const std::vector<model::BlockProperty>& properties)
{
	if (properties.size() != 0)
	{
		return false;
	}
	return true;
}

const std::string_view& node::DerivativeBlockClass::GetDescription() const
{
	return Description;
}

node::BlockType node::DerivativeBlockClass::GetBlockType(const std::vector<model::BlockProperty>& properties)
{
	UNUSED_PARAM(properties);
	return BlockType::Stateful;
}

node::BlockClass::GetFunctorResult node::DerivativeBlockClass::GetFunctor(const std::vector<model::BlockProperty>& properties)
{
	assert(properties.size() == 0);
	UNUSED_PARAM(properties);
	return opt::NLStatefulEquation{
		{0},
		{1},
		opt::NLStatefulEquation::NLStatefulFunctor{[](std::span<const double> in, std::span<double> out, const double t, const opt::FatAny& old_state) ->opt::FatAny
		{
			if (old_state.contains<std::array<double, 3>>())
			{
				const auto& arr = old_state.get<const std::array<double, 3>>();
				if (arr[0] == t)
				{
					out[0] = arr[2];
				}
				else
				{
					out[0] = (in[0] - arr[1]) / (t - arr[0]);
				}
			}
			else
			{
				out[0] = 0;
			}

			return opt::FatAny{ std::array<double, 3>{ t,in[0], out[0] }};
		}}
	};
}

