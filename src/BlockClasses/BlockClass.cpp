#include "BlockClass.hpp"
#include "toolgui/Dialog.hpp"

node::BlockClass::BlockClass(std::string name)
	:m_name{std::move(name)}
{
}

node::BlockClass::~BlockClass()
{
}

std::unique_ptr<node::Dialog> node::BlockClass::CreateBlockDialog(Scene& scene, model::BlockModel& model, std::any& simulation_data)
{
	(void)scene;
	(void)model;
	(void)simulation_data;
	return nullptr;
}