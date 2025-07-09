#pragma once

#include "PluginAPI/BlockClass.hpp"

namespace node
{

	class LuaExpressionClass : public RcBlockClass
	{
	public:
		void GetName(GetNameCallback cb, void* context) const override;
		void GetDescription(GetDescriptionCallback cb, void* context) const override;

		void GetDefaultClassProperties(GetDefaultClassPropertiesCallback cb, void* context) const override;
		int ValidateClassProperties(std::span<const model::BlockProperty> properties, IValidatePropertiesNotifier& error_cb) const override;

		void CalculateSockets(std::span<const model::BlockProperty> properties, ICalculateSocketCallback& cb) const override;
		BlockType GetBlockType(std::span<const model::BlockProperty> properties) const override;

		int GetFunctor(std::span<const model::BlockProperty> properties, IGetFunctorCallback& cb) const override;
	private:

	};
}