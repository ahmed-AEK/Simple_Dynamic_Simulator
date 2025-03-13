#include "RampSourceClass.hpp"

static const std::vector<node::model::BlockProperty> ClassProperties{
	*node::model::BlockProperty::Create("Slope", node::model::BlockPropertyType::FloatNumber, 1.0 )
};
static constexpr std::string_view Description = "Output = Slope * t";

static constexpr node::model::BlockSocketModel::SocketType class_sockets[] = {
		node::model::BlockSocketModel::SocketType::output
};

node::RampSourceClass::RampSourceClass()
	:BuiltinBasicClass{ "Ramp", ClassProperties, class_sockets, Description, BlockType::Source }
{
}

node::BlockClass::GetFunctorResult node::RampSourceClass::GetFunctor(const std::vector<model::BlockProperty>& properties) const
{
	assert(properties.size() == 1);
	assert(std::holds_alternative<double>(properties[0].prop));
	double value = std::get<double>(properties[0].prop);
	return opt::SourceEqWrapper{
		{0},
		opt::make_SourceEqn<opt::FunctorSourceEq>([value](std::span<double> out, const double& t, opt::SourceEvent&)
		{
			out[0] = value * t;
		}),
		{}
	};
}

