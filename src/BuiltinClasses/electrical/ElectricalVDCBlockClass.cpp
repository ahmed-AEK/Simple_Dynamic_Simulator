#include "electrical/ElectricalVDCBlockClass.hpp"

static const std::vector<node::model::BlockProperty> ClassProperties{
	*node::model::BlockProperty::Create("Value", node::model::BlockPropertyType::FloatNumber, 1)
};

static constexpr std::string_view Description = "Basic ideal Electrical DC Voltage Source";

static constexpr node::IBlockClass::SocketIdentification class_sockets[] = {
	{ node::model::BlockSocketModel::SocketType::inout, node::model::NetCategory{"electrical"} },
	{ node::model::BlockSocketModel::SocketType::inout, node::model::NetCategory{"electrical"} },
	{ node::model::BlockSocketModel::SocketType::output, {} }

};

node::ElectricalVDCBlockClass::ElectricalVDCBlockClass()
	:BuiltinBasicClass{ "Electrical VDC", ClassProperties, {}, Description, BlockType::Flow }
{
}


void node::ElectricalVDCBlockClass::CalculateSockets(std::span<const model::BlockProperty> properties, ICalculateSocketCallback& cb) const
{
	UNUSED_PARAM(properties);
	[[maybe_unused]] LightValidatePropertiesNotifier notifier;
	assert(ValidateClassProperties(properties, notifier));
	assert(!notifier.errored);

	cb.add_IdentifiedSockets(class_sockets);
}

int node::ElectricalVDCBlockClass::GetFunctor(std::span<const model::BlockProperty> properties, IGetFunctorCallback& cb) const
{
	assert(properties.size() == 1);
	assert(std::holds_alternative<double>(properties[0].prop));
	double DCValue = std::get<double>(properties[0].prop);
	struct ElectricalVDCBlockFunction : public opt::IPotentialEquation
	{
		explicit ElectricalVDCBlockFunction(double DCValue) : DCValue{ DCValue } {}
		opt::Status Apply(std::span<const double> inputs, double flow, double& potential) override
		{
			UNUSED_PARAM(inputs);
			UNUSED_PARAM(flow);
			potential = -DCValue;
			return opt::Status::ok;
		}
		double DCValue;
	};

	opt::PotentialEquationWrapper eq{
		{},
		{0,1},
		2,
		opt::make_PotentialEqn<ElectricalVDCBlockFunction>(DCValue)
	};
	node::BlockView view{ eq };
	cb.call({ &view,1 });
	return true;
}


