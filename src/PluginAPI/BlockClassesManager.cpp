#include "BlockClassesManager.hpp"

static std::string GetClassName(node::IBlockClass& block)
{
	std::string ret;
	block.GetName([](void* context, std::string_view description) { *static_cast<std::string*>(context) = std::string{ description }; }, &ret);
	return ret;
}

bool node::BlockClassesManager::RegisterBlockClass(BlockClassPtr class_ptr)
{
	std::lock_guard g{ m_mutex };
	assert(class_ptr);
	if (!class_ptr)
	{
		return false;
	}
	auto name = GetClassName(*class_ptr);
	
	auto it = m_classes.find(name);
	if (it != m_classes.end() && it->first == name)
	{
		assert(false);
		return false;
	}
	m_classes.emplace(std::move(name), std::move(class_ptr));
	return true;
}

node::BlockClassPtr node::BlockClassesManager::GetBlockClassByName(const std::string& name)
{
	std::lock_guard g{ m_mutex };
	auto it_class = m_classes.find(name);
	if (it_class == m_classes.end())
	{
		assert(false);
		return nullptr;
	}
	return it_class->second;
}
