#include "electrical/ElectricalControlledIDCBlockClass.hpp"

static const std::vector<node::model::BlockProperty> ClassProperties{
};

static constexpr std::string_view Description = "Basic ideal Electrical Controlled DC Current Source";

static constexpr node::IBlockClass::SocketIdentification class_sockets[] = {
	{ node::model::BlockSocketModel::SocketType::inout, node::model::NetCategory{"electrical"} },
	{ node::model::BlockSocketModel::SocketType::inout, node::model::NetCategory{"electrical"} },
	{ node::model::BlockSocketModel::SocketType::input,  {} },
};

node::ElectricalControlledIDCBlockClass::ElectricalControlledIDCBlockClass()
	:BuiltinBasicClass{ "Electrical Controlled IDC", ClassProperties, {}, Description, BlockType::Flow }
{
}


void node::ElectricalControlledIDCBlockClass::CalculateSockets(std::span<const model::BlockProperty> properties, ICalculateSocketCallback& cb) const
{
	UNUSED_PARAM(properties);
	[[maybe_unused]] LightValidatePropertiesNotifier notifier;
	assert(ValidateClassProperties(properties, notifier));
	assert(!notifier.errored);

	cb.add_IdentifiedSockets(class_sockets);
}

int node::ElectricalControlledIDCBlockClass::GetFunctor(std::span<const model::BlockProperty> properties, IGetFunctorCallback& cb) const
{
	assert(properties.size() == 0);
	UNUSED_PARAM(properties);
	struct ElectricalControlledIDCBlockFunction : public opt::IFlowEquation
	{
		explicit ElectricalControlledIDCBlockFunction() {}
		opt::Status Apply(std::span<const double> input, std::span<double> output) override
		{
			output[0] = -input[0];
			output[1] = input[0];
			return opt::Status::ok;
		}
		double DCValue;
	};

	opt::FlowEquationWrapper eq{
		{2},
		{0,1},
		opt::make_FlowEqn<ElectricalControlledIDCBlockFunction>()
	};
	node::BlockView view{ eq };
	cb.call({ &view,1 });
	return true;
}


