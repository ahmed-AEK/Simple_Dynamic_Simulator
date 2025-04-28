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

void node::BuiltinBasicClass::GetDefaultClassProperties(GetDefaultClassPropertiesCallback cb, void* context) const
{
    cb(context, m_defaultProperties);
}

void node::BuiltinBasicClass::CalculateSockets(std::span<const model::BlockProperty> properties, CalculateSocketCallback cb, void* context) const
{
	UNUSED_PARAM(properties);
	[[maybe_unused]] LightValidatePropertiesNotifier notifier;
	assert(ValidateClassProperties(properties, notifier));
	assert(!notifier.errored);
	cb(context, m_sockets);
}

void node::BuiltinBasicClass::GetDescription(GetDescriptionCallback cb, void* context) const
{
	cb(context, m_description);
}

int node::BuiltinBasicClass::ValidateClassProperties(std::span<const model::BlockProperty> properties, IValidatePropertiesNotifier& error_cb) const
{
	return ValidateEqualPropertyTypes(properties, m_defaultProperties, error_cb);
}

node::BlockType node::BuiltinBasicClass::GetBlockType(std::span<const model::BlockProperty> properties) const
{
	UNUSED_PARAM(properties);
    return m_block_type;
}
