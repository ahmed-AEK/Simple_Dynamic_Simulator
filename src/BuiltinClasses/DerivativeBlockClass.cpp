#include "DerivativeBlockClass.hpp"

static const std::vector<node::model::BlockProperty> ClassProperties{
};

static constexpr std::string_view Description = "output = d(input)/dt";

static constexpr node::model::BlockSocketModel::SocketType class_sockets[] = {
		node::model::BlockSocketModel::SocketType::input,
		node::model::BlockSocketModel::SocketType::output
};

node::DerivativeBlockClass::DerivativeBlockClass()
	:BuiltinBasicClass{ "Derivative", ClassProperties, class_sockets, Description, BlockType::Stateful}
{
}

int node::DerivativeBlockClass::GetFunctor(std::span<const model::BlockProperty> properties, IGetFunctorCallback& cb) const
{
	struct DerivativeState
	{
		double last_input;
		double last_input_time;
		double last_out;
	};
	assert(properties.size() == 0);
	UNUSED_PARAM(properties);

	class DerivativeClassFunction: public opt::INLStatefulEquation
	{
	public:
		opt::Status Apply(std::span<const double> input, std::span<double> output, double t, opt::NLStatefulEquationDataCRef data) override
		{
			UNUSED_PARAM(data);
			if (m_state)
			{
				if (t == m_state->last_input_time)
				{
					output[0] = m_state->last_out;
				}
				else
				{
					output[0] = (input[0] - m_state->last_input) / (t - m_state->last_input_time);
				}
			}
			else
			{
				output[0] = 0;
			}
			return opt::Status::ok;
		}
		opt::Status Update(std::span<const double> input, double t, opt::NLStatefulEquationDataRef data) override
		{
			UNUSED_PARAM(data);
			if (m_state)
			{
				m_state->last_out = (input[0] - m_state->last_input) / (t - m_state->last_input_time);;
				m_state->last_input = input[0];
				m_state->last_input_time = t;
			}
			else
			{
				m_state = DerivativeState{ input[0], t, 0 };
			}
			return opt::Status::ok;
		}
	private:
		std::optional<DerivativeState> m_state;
	};
	opt::NLStatefulEquationWrapper eq{
		{0},
		{1},
		opt::make_NLStatefulEqn<DerivativeClassFunction>(),
		{}
	};
	node::BlockView view{ eq };
	cb.call({ &view,1 });
	return true;
}

