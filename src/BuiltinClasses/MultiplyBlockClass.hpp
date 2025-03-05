#pragma once

#include "PluginAPI/BlockClass.hpp"

namespace node
{

class MultiplyBlockClass : public BlockClass
{
public:
	MultiplyBlockClass();
	
	const std::vector<model::BlockProperty>& GetDefaultClassProperties() override;
	std::vector<model::BlockSocketModel::SocketType>
		CalculateSockets(const std::vector<model::BlockProperty>& properties) override;
	const std::string_view& GetDescription() const override;
	bool ValidateClassProperties(const std::vector<model::BlockProperty>& properties) override;
	BlockType GetBlockType(const std::vector<model::BlockProperty>& properties) override;
	GetFunctorResult GetFunctor(const std::vector<model::BlockProperty>& properties) override;
};

}
