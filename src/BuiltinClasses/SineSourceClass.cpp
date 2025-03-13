#include "SineSourceClass.hpp"

static const std::vector<node::model::BlockProperty> ClassProperties{
	*node::model::BlockProperty::Create("Phase_deg", node::model::BlockPropertyType::FloatNumber, 0.0 ),
	*node::model::BlockProperty::Create("Freq_hz", node::model::BlockPropertyType::FloatNumber, 1.0 ),
};

static constexpr node::model::BlockSocketModel::SocketType class_sockets[] = {
		node::model::BlockSocketModel::SocketType::output
};

static constexpr std::string_view Description = "Output = Sin(2*pi*freq + phase*pi/180)";

node::SineSourceClass::SineSourceClass()
	:BuiltinBasicClass{ "Sine", ClassProperties, class_sockets, Description, BlockType::Source }
{
}

node::BlockClass::GetFunctorResult node::SineSourceClass::GetFunctor(const std::vector<model::BlockProperty>& properties) const
{
	assert(ValidateClassProperties(properties));
	double phase_rad = std::get<double>(properties[0].prop) / 180.0 * std::numbers::pi;
	double freq_rad = std::get<double>(properties[1].prop) * 2 * std::numbers::pi;
	return opt::SourceEqWrapper{
		{0},
		opt::make_SourceEqn<opt::FunctorSourceEq>([phase_rad, freq_rad](std::span<double> out, const double& t, opt::SourceEvent&)
		{
			out[0] = std::sin(t* freq_rad + phase_rad);
		}),
		{}
	};
}

