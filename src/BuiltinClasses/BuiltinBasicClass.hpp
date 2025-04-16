#pragma once

#include "PluginAPI/BlockClass.hpp"

namespace node
{

class BuiltinBasicClass: public BlockClass
{
public:
	BuiltinBasicClass(std::string name, 
		std::span<const node::model::BlockProperty> defaultProperties,
		std::span<const model::BlockSocketModel::SocketType> sockets,
		std::string_view description,
		BlockType block_type);
	void GetDefaultClassProperties(GetDefaultClassPropertiesCallback cb, void* context) const override;
	void CalculateSockets(std::span<const model::BlockProperty> properties, CalculateSocketCallback cb, void* context) const override;
	void GetDescription(GetDescriptionCallback cb, void* context) const override;
	int ValidateClassProperties(std::span<const model::BlockProperty> properties, IValidatePropertiesNotifier& error_cb) const override;
	BlockType GetBlockType(std::span<const model::BlockProperty> properties) const override;
private:
	std::span<const node::model::BlockProperty> m_defaultProperties;
	std::span<const model::BlockSocketModel::SocketType> m_sockets;
	std::string_view m_description;
	BlockType m_block_type;
};

}