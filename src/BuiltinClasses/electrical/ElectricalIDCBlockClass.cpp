#include "electrical/ElectricalIDCBlockClass.hpp"

static const std::vector<node::model::BlockProperty> ClassProperties{
	*node::model::BlockProperty::Create("Value", node::model::BlockPropertyType::FloatNumber, 1)
};

static constexpr std::string_view Description = "Basic ideal Electrical DC Current Source";

static constexpr node::IBlockClass::SocketIdentification class_sockets[] = {
	{ node::model::BlockSocketModel::SocketType::inout, node::model::NetCategory{"electrical"} },
	{ node::model::BlockSocketModel::SocketType::inout, node::model::NetCategory{"electrical"} },
};

node::ElectricalIDCBlockClass::ElectricalIDCBlockClass()
	:BuiltinBasicClass{ "Electrical IDC", ClassProperties, {}, Description, BlockType::Flow }
{
}


void node::ElectricalIDCBlockClass::CalculateSockets(std::span<const model::BlockProperty> properties, ICalculateSocketCallback& cb) const
{
	UNUSED_PARAM(properties);
	[[maybe_unused]] LightValidatePropertiesNotifier notifier;
	assert(ValidateClassProperties(properties, notifier));
	assert(!notifier.errored);

	cb.add_IdentifiedSockets(class_sockets);
}

int node::ElectricalIDCBlockClass::GetFunctor(std::span<const model::BlockProperty> properties, IGetFunctorCallback& cb) const
{
	assert(properties.size() == 1);
	assert(std::holds_alternative<double>(properties[0].prop));
	double DCValue = std::get<double>(properties[0].prop);
	struct ElectricalIDCBlockFunction : public opt::IFlowEquation
	{
		explicit ElectricalIDCBlockFunction(double DCValue) : DCValue{ DCValue } {}
		opt::Status Apply(std::span<const double> input, std::span<double> output) override
		{
			UNUSED_PARAM(input);
			output[0] = -DCValue;
			output[1] = DCValue;
			return opt::Status::ok;
		}
		double DCValue;
	};

	opt::FlowEquationWrapper eq{
		{},
		{0,1},
		opt::make_FlowEqn<ElectricalIDCBlockFunction>(DCValue)
	};
	node::BlockView view{ eq };
	cb.call({ &view,1 });
	return true;
}


