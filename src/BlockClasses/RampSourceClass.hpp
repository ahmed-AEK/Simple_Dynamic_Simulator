#pragma once

#include "BlockClasses/BlockClass.hpp"

namespace node
{

class RampSourceClass : public BlockClass
{
public:
	RampSourceClass();

	const std::vector<model::BlockProperty>& GetClassProperties() override;
	std::vector<model::BlockSocketModel::SocketType>
		CalculateSockets(const std::vector<model::BlockProperty>& properties) override;
	const std::string_view& GetDescription() const override;
	bool ValidateClassProperties(const std::vector<model::BlockProperty>& properties) override;
	BlockType GetBlockType(const std::vector<model::BlockProperty>& properties) override;
	BlockFunctor GetFunctor(const std::vector<model::BlockProperty>& properties) override;
};

}