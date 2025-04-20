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