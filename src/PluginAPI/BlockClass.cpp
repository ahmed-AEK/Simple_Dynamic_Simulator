#include "BlockClass.hpp"
#include "PluginAPI/BlockDialog.hpp"

node::BlockClass::BlockClass(std::string name)
	:m_name{std::move(name)}
{
}

node::BlockClass::~BlockClass()
{
}

std::unique_ptr<node::BlockDialog> node::BlockClass::CreateBlockDialog(Scene& scene, std::shared_ptr<IBlockPropertiesUpdater> model_updater, model::BlockModel& model,
	model::FunctionalBlockData& data, std::any& simulation_data) {
	(void)scene;
	(void)model;
	(void)simulation_data;
	(void)data;
	(void)model_updater;
	return nullptr;
};

int node::ValidateEqualPropertyTypes(std::span<const model::BlockProperty> properties, std::span<const model::BlockProperty> default_properties, IBlockClass::IValidatePropertiesNotifier& error_cb)
{
	if (properties.size() != default_properties.size())
	{
		error_cb.error(0, std::format("size mismatch, expected: {}, got: {}", default_properties.size(), properties.size()));
		return false;
	}
	for (size_t i = 0; i < properties.size(); i++)
	{
		if (properties[i].name != default_properties[i].name)
		{
			error_cb.error(i, std::format("property name mismatch, expected: {}, got: {}", default_properties[i].name, properties[i].name));
			return false;
		}
		if (properties[i].GetType() != default_properties[i].GetType())
		{
			error_cb.error(i, std::format("property type mismatch"));
			return false;
		}
	}
	return true;
}
