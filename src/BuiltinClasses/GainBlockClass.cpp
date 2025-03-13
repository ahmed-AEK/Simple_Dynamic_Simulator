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

node::BlockClass::GetFunctorResult node::GainBlockClass::GetFunctor(const std::vector<model::BlockProperty>& properties) const
{
	assert(properties.size() == 1);
	assert(std::holds_alternative<double>(properties[0].prop));
	double multiplier = std::get<double>(properties[0].prop);
	struct GainBlockFunction: public opt::INLEquation
	{
		GainBlockFunction(double mul) : multiplier{mul} {}
		virtual void Apply(std::span<const double> input, std::span<double> output)
		{
			output[0] = input[0] * multiplier;
		}
		double multiplier;
	};
	return opt::NLEquationWrapper{
		{0},
		{1},
		opt::make_NLEqn<GainBlockFunction>(multiplier)
	};
}

