#pragma once

#include "PluginAPI/BlockClass.hpp"
#include <map>
namespace node
{

class BlockClassesManager
{
public:
	enum class ClassId : uint32_t {};

	bool RegisterBlockClass(const std::shared_ptr<BlockClass>& class_ptr);
	std::shared_ptr<BlockClass> GetBlockClassByName(const std::string& name);
private:
	std::unordered_map<std::string, std::shared_ptr<BlockClass>> m_classes;
};

}