#include "DerivativeBlockClass.hpp"

static const std::vector<node::model::BlockProperty> ClassProperties{
};

static constexpr std::string_view Description = "output = d(input)/dt";


node::DerivativeBlockClass::DerivativeBlockClass()
	:BlockClass("Derivative")
{
}

const std::vector<node::model::BlockProperty>& node::DerivativeBlockClass::GetDefaultClassProperties()
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
	struct DerivativeState
	{
		double last_input;
		double last_input_time;
		double last_out;
	};
	assert(properties.size() == 0);
	UNUSED_PARAM(properties);
	return opt::NLStatefulEquation{
		{0},
		{1},
		opt::NLStatefulEquation::NLStatefulFunctor{[](std::span<const double> in, std::span<double> out, const double t, const opt::NLStatefulEquation& eq)
		{
			const auto& old_state = eq.GetState();
			if (old_state.contains<DerivativeState>())
			{
				const auto& state = old_state.get<const DerivativeState>();
				if (t == state.last_input_time)
				{
					out[0] = state.last_out;
				}
				else
				{
					out[0] = (in[0] - state.last_input) / (t - state.last_input_time);
				}
			}
			else
			{
				out[0] = 0;
			}
		}},
		opt::NLStatefulEquation::NLStatefulUpdateFunctor{[](std::span<const double> in, const double t, opt::NLStatefulEquation& eq)
		{
			auto& old_state = eq.GetState();
			if (old_state.contains<DerivativeState>())
			{
				auto& state = old_state.get<DerivativeState>();
				state.last_out = (in[0] - state.last_input) / (t - state.last_input_time);;
				state.last_input = in[0];
				state.last_input_time = t;
			}
			else
			{
				old_state = opt::FatAny{DerivativeState{in[0], t, 0}};
			}
		}
		}
	};
}

