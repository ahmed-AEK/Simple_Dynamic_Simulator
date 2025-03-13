#include "IntegrationBlockClass.hpp"

static const std::vector<node::model::BlockProperty> ClassProperties{
};

static constexpr std::string_view Description = "output = integration(input)";
static constexpr node::model::BlockSocketModel::SocketType class_sockets[] = {
		node::model::BlockSocketModel::SocketType::input,
		node::model::BlockSocketModel::SocketType::output
};


node::IntegrationBlockClass::IntegrationBlockClass()
	:BuiltinBasicClass{ "Integration", ClassProperties, class_sockets, Description, BlockType::Stateless }
{
}

node::BlockClass::GetFunctorResult node::IntegrationBlockClass::GetFunctor(const std::vector<model::BlockProperty>& properties) const
{
	assert(properties.size() == 0);
	UNUSED_PARAM(properties);
	struct SimpleIntegrator : public opt::IDiffEquation
	{
		void Apply(std::span<const double> input, std::span<double> output, const double t) override
		{
			UNUSED_PARAM(t);
			output[0] = input[0];
		}
	};
	return opt::DiffEquationWrapper{
		{0},
		{1},
		opt::make_DiffEqn<SimpleIntegrator>()
	};
}

