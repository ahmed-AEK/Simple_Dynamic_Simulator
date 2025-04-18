#include "AddSimpleClass.hpp"

static const std::vector<node::model::BlockProperty> ClassProperties{
};

static constexpr std::string_view Description = "output = input1 + input2";

static constexpr node::model::BlockSocketModel::SocketType class_sockets[] = {
		node::model::BlockSocketModel::SocketType::input,
		node::model::BlockSocketModel::SocketType::input,
		node::model::BlockSocketModel::SocketType::output
};

node::AddSimpleBlockClass::AddSimpleBlockClass()
	:BuiltinBasicClass{ "Add Simple" , ClassProperties, class_sockets, Description, BlockType::Stateless }
{
}

int node::AddSimpleBlockClass::GetFunctor(std::span<const model::BlockProperty> properties, IGetFunctorCallback& cb) const
{
	assert(properties.size() == 0);
	UNUSED_PARAM(properties);
	struct AddBlockFunction : public opt::INLEquation
	{
		AddBlockFunction() {}
		opt::Status Apply(std::span<const double> input, std::span<double> output) override
		{
			output[0] = input[0] + input[1];
			return opt::Status::ok;
		}
		void Destroy() override
		{
			// do nothing
		}
	};
	static AddBlockFunction block_fn;
	opt::NLEquationWrapper eq{
		{0,1},
		{2},
		opt::NLEqPtr{&block_fn}
	};
	node::BlockView view{ eq };
	cb.call({ &view,1 });
	return true;
}

