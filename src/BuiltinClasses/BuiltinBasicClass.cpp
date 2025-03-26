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

std::vector<node::model::SocketType> node::BuiltinBasicClass::CalculateSockets(const std::vector<model::BlockProperty>& properties) const
{
	UNUSED_PARAM(properties);
	[[maybe_unused]] LightValidatePropertiesNotifier notifier;
	assert(ValidateClassProperties(properties, notifier));
	assert(!notifier.errored);
    return { m_sockets.begin(), m_sockets.end() };
}

std::string_view node::BuiltinBasicClass::GetDescription() const
{
    return m_description;
}

bool node::BuiltinBasicClass::ValidateClassProperties(const std::vector<model::BlockProperty>& properties, IValidatePropertiesNotifier& error_cb) const
{
	if (properties.size() != m_defaultProperties.size())
	{
		error_cb.error(0, std::format("size mismatch, expected: {}, got: {}", m_defaultProperties.size(), properties.size()));
		return false;
	}
	for (size_t i = 0; i < properties.size(); i++)
	{
		if (properties[i].name != m_defaultProperties[i].name)
		{
			error_cb.error(i, std::format("property name mismatch, expected: {}, got: {}", m_defaultProperties[i].name, properties[i].name));
			return false;
		}
		if (properties[i].GetType() != m_defaultProperties[i].GetType())
		{
			error_cb.error(i, std::format("property type mismatch"));
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
