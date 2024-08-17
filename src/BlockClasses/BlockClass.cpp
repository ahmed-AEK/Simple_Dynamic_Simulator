#include "BlockClass.hpp"

node::BlockClass::BlockClass(std::string name)
	:m_name{std::move(name)}
{
}

node::BlockClass::~BlockClass()
{
}
