#pragma once

#include "PluginAPI/BlockClass.hpp"
#include <map>
namespace node
{

class BlockClassesManager
{
public:
	enum class ClassId : uint32_t {};

	bool RegisterBlockClass(BlockClassPtr class_ptr);
	BlockClassPtr GetBlockClassByName(const std::string& name);
private:
	std::unordered_map<std::string, BlockClassPtr> m_classes;
};

}