#include "ScopeDisplayClass.hpp"

#include "toolgui/NodeMacros.h"
#include "toolgui/Dialog.hpp"

#include "optimizer/NLEquation.hpp"
#include "optimizer/DiffEquation.hpp"
#include "optimizer/NLStatefulEquation.hpp"
#include "optimizer/Observer.hpp"
#include "optimizer/SourceEq.hpp"
#include "ScopeDisplayDialog.hpp"

static const std::vector<node::model::BlockProperty> ClassProperties{
	node::model::BlockProperty{"Inputs", node::model::BlockPropertyType::UnsignedInteger, static_cast<uint64_t>(1)}
};

static constexpr std::string_view Description = "Displays Input signals";

node::ScopeDisplayClass::ScopeDisplayClass()
	:BlockClass("Scope Display")
{
}

const std::vector<node::model::BlockProperty>& node::ScopeDisplayClass::GetClassProperties()
{
	return ClassProperties;
}

std::vector<node::model::BlockSocketModel::SocketType> node::ScopeDisplayClass::CalculateSockets(const std::vector<model::BlockProperty>& properties)
{
	UNUSED_PARAM(properties);
	assert(ValidateClassProperties(properties));
	return {
		node::model::BlockSocketModel::SocketType::input
	};
}

bool node::ScopeDisplayClass::ValidateClassProperties(const std::vector<model::BlockProperty>& properties)
{
	if (properties.size() != 1)
	{
		return false;
	}
	if (properties[0].name != ClassProperties[0].name)
	{
		return false;
	}
	if (properties[0].type != ClassProperties[0].type)
	{
		return false;
	}
	if (!std::holds_alternative<uint64_t>(properties[0].prop))
	{
		return false;
	}
	return true;
}

const std::string_view& node::ScopeDisplayClass::GetDescription() const
{
	return Description;
}

node::BlockType node::ScopeDisplayClass::GetBlockType(const std::vector<model::BlockProperty>& properties)
{
	UNUSED_PARAM(properties);
	return BlockType::Sink;
}

node::BlockFunctor node::ScopeDisplayClass::GetFunctor(const std::vector<model::BlockProperty>& properties)
{
	assert(properties.size() == 1);
	assert(std::holds_alternative<uint64_t>(properties[0].prop));
	uint64_t value = std::get<uint64_t>(properties[0].prop);
	std::pmr::vector<int64_t> ports;
	for (uint64_t i = 0; i < value; i++)
	{
		ports.push_back(i);
	}
	std::shared_ptr<std::vector<std::vector<double>>> vec = std::make_shared<std::vector<std::vector<double>>>();
	for (int i = 0; i < value + 1; i++)
	{
		vec->push_back({});
	}
	return opt::Observer{
		ports,
		opt::Observer::ObserverFunctor{[vec](std::span<const double> out, const double& t)
		{
			for (size_t i = 0; i < out.size(); i++)
			{
				(*vec)[i].push_back(out[i]);
			}
			(*vec)[out.size()].push_back(t);
		}}
	};
}

std::unique_ptr<node::Dialog> node::ScopeDisplayClass::CreateBlockDialog(Scene& scene, model::BlockModel& model, std::any& simulation_data)
{
	UNUSED_PARAM(model);
	UNUSED_PARAM(simulation_data);
	auto dialog = std::make_unique<ScopeDiplayDialog>(SDL_Rect{ 100,100, 500,500 }, &scene);
	return dialog;
}

bool node::ScopeDisplayClass::HasBlockDialog() const
{
	return true;
}





