#include "BlockClassesManager.hpp"

bool node::BlockClassesManager::RegisterBlockClass(const std::shared_ptr<BlockClass>& class_ptr)
{
	assert(class_ptr);
	if (!class_ptr)
	{
		return false;
	}
	auto name = std::string{ class_ptr->GetName() };
	
	auto it = m_classes.find(name);
	if (it != m_classes.end() && it->first == name)
	{
		assert(false);
		return false;
	}
	m_classes.emplace(std::move(name), class_ptr);
	return true;
}

std::shared_ptr<node::BlockClass> node::BlockClassesManager::GetBlockClassByName(const std::string& name)
{
	auto it_class = m_classes.find(name);
	if (it_class == m_classes.end())
	{
		assert(false);
		return nullptr;
	}
	return it_class->second;
}
