#pragma once

#include "PluginAPI/BlockClass.hpp"

namespace node
{

	class LuaStatefulEqnClass: public RcBlockClass
	{
	public:
		std::string_view GetName() const override;
		std::string_view GetDescription() const override;

		std::span<const model::BlockProperty> GetDefaultClassProperties() const override;
		bool ValidateClassProperties(const std::vector<model::BlockProperty>& properties, IValidatePropertiesNotifier& error_cb) const override;

		std::vector<model::SocketType>
			CalculateSockets(const std::vector<model::BlockProperty>& properties) const override;
		BlockType GetBlockType(const std::vector<model::BlockProperty>& properties) const override;

		GetFunctorResult GetFunctor(const std::vector<model::BlockProperty>& properties) const override;
	private:

	};
}