#include "electrical/ElectricalResistorBlockClass.hpp"

static const std::vector<node::model::BlockProperty> ClassProperties{
	*node::model::BlockProperty::Create("Resistance", node::model::BlockPropertyType::FloatNumber, 100)
};

static constexpr std::string_view Description = "Basic ideal Electrical Resistor";
using namespace std::literals::string_view_literals;

static constexpr node::IBlockClass::SocketIdentification class_sockets[] = {
	{ node::model::BlockSocketModel::SocketType::inout, node::model::NetCategory{"electrical"} },
	{ node::model::BlockSocketModel::SocketType::inout, node::model::NetCategory{"electrical"} }
};

node::ElectricalResistorBlockClass::ElectricalResistorBlockClass()
	:BuiltinBasicClass{ "Electrical Resistor", ClassProperties, {}, Description, BlockType::Flow }
{
}


void node::ElectricalResistorBlockClass::CalculateSockets(std::span<const model::BlockProperty> properties, ICalculateSocketCallback& cb) const
{
	UNUSED_PARAM(properties);
	[[maybe_unused]] LightValidatePropertiesNotifier notifier;
	assert(ValidateClassProperties(properties, notifier));
	assert(!notifier.errored);
	
	cb.add_IdentifiedSockets(class_sockets);
}

int node::ElectricalResistorBlockClass::GetFunctor(std::span<const model::BlockProperty> properties, IGetFunctorCallback& cb) const
{
	assert(properties.size() == 1);
	assert(std::holds_alternative<double>(properties[0].prop));
	double resistance = std::get<double>(properties[0].prop);
	struct ElectricalResistanceBlockFunction : public opt::IFlowEquation
	{
		explicit ElectricalResistanceBlockFunction(double resistance) : conductance{ 1 / resistance } {}
		opt::Status Apply(std::span<const double> inputs, std::span<double> outputs) override
		{
			outputs[0] = (inputs[0] - inputs[1]) * conductance;
			outputs[1] = -(inputs[0] - inputs[1]) * conductance;
			return opt::Status::ok;
		}
		double conductance;
	};

	opt::FlowEquationWrapper eq{
		{0,1},
		{0,1},
		opt::make_FlowEqn<ElectricalResistanceBlockFunction>(resistance)
	};
	node::BlockView view{ eq };
	cb.call({ &view,1 });
	return true;
}


