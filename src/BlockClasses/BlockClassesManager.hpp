#pragma once

#include "BlockClasses/BlockClass.hpp"
#include <map>
namespace node
{

class BlockClassesManager
{
public:
	bool RegisterBlockClass(const std::shared_ptr<BlockClass>& class_ptr);
	std::shared_ptr<BlockClass> GetBlockClassByName(const std::string& name);
private:
	std::map<std::string, std::shared_ptr<BlockClass>> m_classes;

};

}