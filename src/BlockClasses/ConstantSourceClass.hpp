#pragma once

#include "BlockClasses/BlockClass.hpp"

namespace node
{

class ConstantSourceClass : public BlockClass
{
public:
	ConstantSourceClass();

	const std::vector<model::BlockProperty>& GetClassProperties() override;
	std::vector<model::BlockSocketModel::SocketType>
		CalculateSockets(const std::vector<model::BlockProperty>& properties) override;
	bool ValidateClassProperties(const std::vector<model::BlockProperty>& properties) override;
	BlockType GetBlockType(const std::vector<model::BlockProperty>& properties) override;
	BlockFunctor GetFunctor(const std::vector<model::BlockProperty>& properties) override;
};

}