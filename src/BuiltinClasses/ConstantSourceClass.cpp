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

int node::ConstantSourceClass::GetFunctor(std::span<const model::BlockProperty> properties, IGetFunctorCallback& cb) const
{
	assert(properties.size() == 1);
	assert(std::holds_alternative<double>(properties[0].prop));
	double value = std::get<double>(properties[0].prop);
	opt::SourceEqWrapper eq{
		{0},
		opt::make_SourceEqn<opt::FunctorSourceEq>([value](std::span<double> out, const double&, opt::SourceEvent&)
		{
			out[0] = value;
		}),
		{}
	};
	node::BlockView view{ eq };
	cb.call({ &view,1 });
	return true;
}

