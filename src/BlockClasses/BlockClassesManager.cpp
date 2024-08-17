#include "BlockClassesManager.hpp"

bool node::BlockClassesManager::RegisterBlockClass(const std::shared_ptr<BlockClass>& class_ptr)
{
	assert(class_ptr);
	if (!class_ptr)
	{
		return false;
	}
	const auto& name = class_ptr->GetName();
	auto it = m_classes.lower_bound(name);
	if (it != m_classes.end() && it->first == name)
	{
		assert(false);
		return false;
	}
	m_classes.emplace_hint(it, name, class_ptr);
	return true;
}

std::shared_ptr<node::BlockClass> node::BlockClassesManager::GetBlockByName(const std::string& name)
{
	auto it = m_classes.find(name);
	if (it != m_classes.end())
	{
		return it->second;
	}
	return nullptr;
}
