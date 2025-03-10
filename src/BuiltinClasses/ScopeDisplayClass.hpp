#pragma once

#include "PluginAPI/BlockClass.hpp"

namespace node
{

class ScopeDisplayClass : public BlockClass
{
public:
	ScopeDisplayClass();
	
	const std::vector<model::BlockProperty>& GetDefaultClassProperties() override;
	std::vector<model::BlockSocketModel::SocketType>
		CalculateSockets(const std::vector<model::BlockProperty>& properties) override;
	const std::string_view& GetDescription() const override;
	bool ValidateClassProperties(const std::vector<model::BlockProperty>& properties) override;
	BlockType GetBlockType(const std::vector<model::BlockProperty>& properties) override;
	GetFunctorResult GetFunctor(const std::vector<model::BlockProperty>& properties) override;

	std::unique_ptr<BlockDialog> CreateBlockDialog(Scene& scene, model::BlockModel& model, 
		model::FunctionalBlockData& data, std::any& simulation_data) override;
	bool HasBlockDialog() const override;
};

}
