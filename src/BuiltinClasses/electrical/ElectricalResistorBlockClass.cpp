#include "electrical/ElectricalResistorBlockClass.hpp"

static const std::vector<node::model::BlockProperty> ClassProperties{
	*node::model::BlockProperty::Create("Resistance", node::model::BlockPropertyType::FloatNumber, 100)
};

static constexpr std::string_view Description = "Basic ideal Electrical Resistor";

static constexpr node::model::BlockSocketModel::SocketType class_sockets[] = {
		node::model::BlockSocketModel::SocketType::inout,
		node::model::BlockSocketModel::SocketType::inout,
};

node::ElectricalResistorBlockClass::ElectricalResistorBlockClass()
	:BuiltinBasicClass{ "Electrical Resistor", ClassProperties, class_sockets, Description, BlockType::Flow }
{
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

