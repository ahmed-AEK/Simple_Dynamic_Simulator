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
	std::span<const model::BlockProperty> GetDefaultClassProperties() const override;
	std::vector<model::SocketType>
		CalculateSockets(const std::vector<model::BlockProperty>& properties) const override;
	std::string_view GetDescription() const override;
	bool ValidateClassProperties(const std::vector<model::BlockProperty>& properties, IValidatePropertiesNotifier& error_cb) const override;
	BlockType GetBlockType(const std::vector<model::BlockProperty>& properties) const override;
private:
	std::span<const node::model::BlockProperty> m_defaultProperties;
	std::span<const model::BlockSocketModel::SocketType> m_sockets;
	std::string_view m_description;
	BlockType m_block_type;
};

}