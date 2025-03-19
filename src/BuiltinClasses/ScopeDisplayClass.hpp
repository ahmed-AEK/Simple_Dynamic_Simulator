#pragma once

#include "PluginAPI/BlockClass.hpp"
#include "BuiltinClasses/BuiltinBasicClass.hpp"

namespace node
{

class ScopeDisplayClass : public BuiltinBasicClass
{
public:
	ScopeDisplayClass();
	std::vector<model::SocketType>
		CalculateSockets(const std::vector<model::BlockProperty>& properties) const override;
	GetFunctorResult GetFunctor(const std::vector<model::BlockProperty>& properties) const override;

	std::unique_ptr<BlockDialog> CreateBlockDialog(Scene& scene, model::BlockModel& model, 
		model::FunctionalBlockData& data, std::any& simulation_data) override;
	bool HasBlockDialog() const override;
};

}
