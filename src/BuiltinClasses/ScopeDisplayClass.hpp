#pragma once

#include "PluginAPI/BlockClass.hpp"
#include "BuiltinClasses/BuiltinBasicClass.hpp"

namespace node
{

class ScopeDisplayClass : public BuiltinBasicClass
{
public:
	ScopeDisplayClass();
	void CalculateSockets(std::span<const model::BlockProperty> properties, ICalculateSocketCallback& cb) const override;
	int GetFunctor(std::span<const model::BlockProperty> properties, IGetFunctorCallback& cb) const override;

	std::unique_ptr<BlockDialog> CreateBlockDialog(Scene& scene, std::shared_ptr<IBlockPropertiesUpdater> model_updater, model::BlockModel& model,
		model::FunctionalBlockData& data, std::any& simulation_data) override;
	bool HasBlockDialog() const override;
};

}
