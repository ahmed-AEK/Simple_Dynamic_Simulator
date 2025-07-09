#pragma once

#include "PluginAPI/BlockClass.hpp"
#include "PluginAPI/Logger.hpp"

namespace node
{

	class LuaStandaloneDiffEqnClass: public BlockClass
	{
	public:
		LuaStandaloneDiffEqnClass();
		void GetName(GetNameCallback cb, void* context) const override;
		void GetDescription(GetDescriptionCallback cb, void* context) const override;

		void GetDefaultClassProperties(GetDefaultClassPropertiesCallback cb, void* context) const override;
		int ValidateClassProperties(std::span<const model::BlockProperty> properties, IValidatePropertiesNotifier& error_cb) const override;

		void CalculateSockets(std::span<const model::BlockProperty> properties, ICalculateSocketCallback& cb) const override;
		BlockType GetBlockType(std::span<const model::BlockProperty> properties) const override;

		int GetFunctor(std::span<const model::BlockProperty> properties, IGetFunctorCallback& cb) const override;

		bool HasBlockDialog() const override;

		std::unique_ptr<BlockDialog> CreateBlockDialog(Scene& scene, std::shared_ptr<IBlockPropertiesUpdater> model_updater, model::BlockModel& model,
			model::FunctionalBlockData& data, std::any& simulation_data) override;

		static const std::string_view DEFAULT_CODE;
	private:
		tl::expected<std::monostate, std::string> validate_lua_script(const std::string& code) const;
		logging::Logger m_logger = logger(logging::LogCategory::Extension);
	};
}