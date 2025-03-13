#include "BuiltinBasicClass.hpp"

node::BuiltinBasicClass::BuiltinBasicClass(std::string name,std::span<const node::model::BlockProperty> defaultProperties,
	std::span<const model::BlockSocketModel::SocketType> sockets, std::string_view description, BlockType block_type)
	: BlockClass{std::move(name)},
	m_defaultProperties{ defaultProperties },
	m_sockets{sockets},
	m_description{description},
	m_block_type{block_type}
{
}

std::span<const node::model::BlockProperty> node::BuiltinBasicClass::GetDefaultClassProperties() const
{
    return m_defaultProperties;
}

std::vector<node::model::BlockSocketModel::SocketType> node::BuiltinBasicClass::CalculateSockets(const std::vector<model::BlockProperty>& properties) const
{
	UNUSED_PARAM(properties);
	assert(ValidateClassProperties(properties));
    return { m_sockets.begin(), m_sockets.end() };
}

std::string_view node::BuiltinBasicClass::GetDescription() const
{
    return m_description;
}

bool node::BuiltinBasicClass::ValidateClassProperties(const std::vector<model::BlockProperty>& properties) const
{
	if (properties.size() != m_defaultProperties.size())
	{
		return false;
	}
	for (size_t i = 0; i < properties.size(); i++)
	{
		if (properties[i].name != m_defaultProperties[i].name)
		{
			return false;
		}
		if (properties[i].GetType() != m_defaultProperties[i].GetType())
		{
			return false;
		}
		if (!(properties[i].prop.index() == m_defaultProperties[i].prop.index()))
		{
			return false;
		}
	}
	return true;
}

node::BlockType node::BuiltinBasicClass::GetBlockType(const std::vector<model::BlockProperty>& properties) const
{
	UNUSED_PARAM(properties);
    return m_block_type;
}
