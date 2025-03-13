#include "ConstantSourceClass.hpp"

static const std::vector<node::model::BlockProperty> ClassProperties{
	*node::model::BlockProperty::Create("Value", node::model::BlockPropertyType::FloatNumber, 1)
};
static constexpr std::string_view Description = "Output = Value";

static constexpr node::model::BlockSocketModel::SocketType class_sockets[] = {
		node::model::BlockSocketModel::SocketType::output
};


node::ConstantSourceClass::ConstantSourceClass()
	:BuiltinBasicClass{"Constant Source", ClassProperties, class_sockets, Description, BlockType::Source}
{
}

node::BlockClass::GetFunctorResult node::ConstantSourceClass::GetFunctor(const std::vector<model::BlockProperty>& properties) const
{
	assert(properties.size() == 1);
	assert(std::holds_alternative<double>(properties[0].prop));
	double value = std::get<double>(properties[0].prop);
	return opt::SourceEqWrapper{
		{0},
		opt::make_SourceEqn<opt::FunctorSourceEq>([value](std::span<double> out, const double&, opt::SourceEvent&)
		{
			out[0] = value;
		}),
		{}
	};
}

