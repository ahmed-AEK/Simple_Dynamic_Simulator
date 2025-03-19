#include "ScopeDisplayClass.hpp"

#include "toolgui/NodeMacros.h"
#include "toolgui/Dialog.hpp"

#include "ScopeDisplayDialog.hpp"

static const std::vector<node::model::BlockProperty> ClassProperties{
	*node::model::BlockProperty::Create("Inputs", node::model::BlockPropertyType::UnsignedInteger, static_cast<uint64_t>(1))
};

static constexpr std::string_view Description = "Displays Input signals, Double Tap to show the Scope Dialog";

node::ScopeDisplayClass::ScopeDisplayClass()
	:BuiltinBasicClass{ "Scope Display", ClassProperties, {}, Description, BlockType::Sink }
{
}

std::vector<node::model::SocketType> node::ScopeDisplayClass::CalculateSockets(const std::vector<model::BlockProperty>& properties) const
{
	UNUSED_PARAM(properties);
	assert(ValidateClassProperties(properties));
	uint64_t count = std::get<uint64_t>(properties[0].prop);
	std::vector<node::model::BlockSocketModel::SocketType> result;
	result.reserve(count);
	for (uint64_t i = 0; i < count; i++)
	{
		result.push_back(node::model::BlockSocketModel::SocketType::input);
	}
	return result;
}

node::BlockClass::GetFunctorResult node::ScopeDisplayClass::GetFunctor(const std::vector<model::BlockProperty>& properties) const
{
	assert(properties.size() == 1);
	assert(std::holds_alternative<uint64_t>(properties[0].prop));
	uint64_t value = std::get<uint64_t>(properties[0].prop);
	std::vector<int32_t> ports;
	for (uint32_t i = 0; i < value; i++)
	{
		ports.push_back(i);
	}
	std::shared_ptr<std::vector<std::vector<double>>> vec = std::make_shared<std::vector<std::vector<double>>>();
	for (uint64_t i = 0; i < value + 1; i++)
	{
		vec->push_back({});
	}
	return opt::ObserverWrapper{
		std::move(ports),
		opt::make_ObserverEqn<opt::FunctorObserver>(opt::FunctorObserver{opt::FunctorObserver::ObserverFunctor{[vec](std::span<const double> out, const double& t)
		{
			for (size_t i = 0; i < out.size(); i++)
			{
				(*vec)[i].push_back(out[i]);
			}
			(*vec)[out.size()].push_back(t);
		}},
		{}, {},
		opt::FunctorObserver::GetResultsFunctor{[vec]()
		{
			return std::any{*vec};
		}}})
	};
}

std::unique_ptr<node::BlockDialog> node::ScopeDisplayClass::CreateBlockDialog(Scene& scene, model::BlockModel& model, 
	model::FunctionalBlockData& data, std::any& simulation_data)
{
	UNUSED_PARAM(model);
	UNUSED_PARAM(data);

	auto dialog = std::make_unique<ScopeDiplayDialog>(WidgetSize{ 500.0f,500.0f }, &scene);

	try
	{
		const auto& sim_data = std::any_cast<const std::vector<std::vector<double>>&>(simulation_data);
		XYSeries xydata;
		assert(sim_data.size() >= 2);
		const size_t time_index = sim_data.size() - 1;
		for (size_t i = 0; i < sim_data[1].size(); i++)
		{
			xydata.points.push_back(SDL_FPoint{ static_cast<float>(sim_data[time_index][i]), static_cast<float>(sim_data[0][i]) });
		}
		dialog->SetData(std::move(xydata));
	}
	catch (std::bad_cast&)
	{
		// do nothing
	}
	return dialog;
}

bool node::ScopeDisplayClass::HasBlockDialog() const
{
	return true;
}





