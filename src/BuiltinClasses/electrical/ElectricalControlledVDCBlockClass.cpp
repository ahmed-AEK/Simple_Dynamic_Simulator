#include "electrical/ElectricalControlledVDCBlockClass.hpp"

static const std::vector<node::model::BlockProperty> ClassProperties{
};

static constexpr std::string_view Description = "Basic ideal Electrical Controlled DC Voltage Source";

static constexpr node::IBlockClass::SocketIdentification class_sockets[] = {
	{ node::model::BlockSocketModel::SocketType::inout, node::model::NetCategory{"electrical"} },
	{ node::model::BlockSocketModel::SocketType::inout, node::model::NetCategory{"electrical"} },
	{ node::model::BlockSocketModel::SocketType::input,  {} },
	{ node::model::BlockSocketModel::SocketType::output, {} }
};

node::ElectricalControlledVDCBlockClass::ElectricalControlledVDCBlockClass()
	:BuiltinBasicClass{ "Electrical Controlled VDC", ClassProperties, {}, Description, BlockType::Flow }
{
}


void node::ElectricalControlledVDCBlockClass::CalculateSockets(std::span<const model::BlockProperty> properties, ICalculateSocketCallback& cb) const
{
	UNUSED_PARAM(properties);
	[[maybe_unused]] LightValidatePropertiesNotifier notifier;
	assert(ValidateClassProperties(properties, notifier));
	assert(!notifier.errored);

	cb.add_IdentifiedSockets(class_sockets);
}

int node::ElectricalControlledVDCBlockClass::GetFunctor(std::span<const model::BlockProperty> properties, IGetFunctorCallback& cb) const
{
	assert(properties.size() == 0);
	UNUSED_PARAM(properties);
	struct ElectricalControlledVDCBlockFunction : public opt::IPotentialEquation
	{
		explicit ElectricalControlledVDCBlockFunction() {}
		opt::Status Apply(std::span<const double> inputs, double flow, double& potential) override
		{
			UNUSED_PARAM(flow);
			potential = -inputs[0];
			return opt::Status::ok;
		}
	};

	opt::PotentialEquationWrapper eq{
		{2},
		{0,1},
		3,
		opt::make_PotentialEqn<ElectricalControlledVDCBlockFunction>()
	};
	node::BlockView view{ eq };
	cb.call({ &view,1 });
	return true;
}


