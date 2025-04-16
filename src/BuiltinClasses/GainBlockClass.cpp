#include "GainBlockClass.hpp"

static const std::vector<node::model::BlockProperty> ClassProperties{
	*node::model::BlockProperty::Create("Multiplier", node::model::BlockPropertyType::FloatNumber, 1)
};

static constexpr std::string_view Description = "Sets its output to its input multiplied by \"Multiplier\"";

static constexpr node::model::BlockSocketModel::SocketType class_sockets[] = {
		node::model::BlockSocketModel::SocketType::input,
		node::model::BlockSocketModel::SocketType::output
};

node::GainBlockClass::GainBlockClass()
	:BuiltinBasicClass{ "Gain", ClassProperties, class_sockets, Description, BlockType::Stateless }
{
}

int node::GainBlockClass::GetFunctor(std::span<const model::BlockProperty> properties, IGetFunctorCallback& cb) const
{
	assert(properties.size() == 1);
	assert(std::holds_alternative<double>(properties[0].prop));
	double multiplier = std::get<double>(properties[0].prop);
	struct GainBlockFunction: public opt::INLEquation
	{
		explicit GainBlockFunction(double mul) : multiplier{mul} {}
		opt::Status Apply(std::span<const double> input, std::span<double> output) override
		{
			output[0] = input[0] * multiplier;
			return opt::Status::ok;
		}
		double multiplier;
	};

	opt::NLEquationWrapper eq{
		{0},
		{1},
		opt::make_NLEqn<GainBlockFunction>(multiplier)
	};
	node::BlockView view{ eq };
	cb.call({&view,1});
	return true;
}

