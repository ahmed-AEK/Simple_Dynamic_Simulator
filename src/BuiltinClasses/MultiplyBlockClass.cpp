#include "MultiplyBlockClass.hpp"

static const std::vector<node::model::BlockProperty> ClassProperties{
};

static constexpr std::string_view Description = "output = input1 * input2";

static constexpr node::model::BlockSocketModel::SocketType class_sockets[] = {
		node::model::BlockSocketModel::SocketType::input,
		node::model::BlockSocketModel::SocketType::input,
		node::model::BlockSocketModel::SocketType::output
};

node::MultiplyBlockClass::MultiplyBlockClass()
	:BuiltinBasicClass{ "Multiply", ClassProperties, class_sockets, Description, BlockType::Stateless }
{
}

node::BlockClass::GetFunctorResult node::MultiplyBlockClass::GetFunctor(const std::vector<model::BlockProperty>& properties) const
{
	assert(properties.size() == 0);
	UNUSED_PARAM(properties);
	struct MultiplyBlockFunction : public opt::INLEquation
	{
		virtual void Apply(std::span<const double> input, std::span<double> output)
		{
			output[0] = input[0] * input[1];
		}
		void Destroy() override
		{

		}
	};
	static MultiplyBlockFunction block_fn;
	return opt::NLEquationWrapper{
		{0,1},
		{2},
		opt::NLEqPtr{&block_fn}
	};
}

