#pragma once

#include "PluginAPI/BlockClass.hpp"
#include "BuiltinClasses/BuiltinBasicClass.hpp"

namespace node
{

	class StepSourceClass : public BuiltinBasicClass
	{
	public:
		StepSourceClass();
		int GetFunctor(std::span<const model::BlockProperty> properties, IGetFunctorCallback& cb) const override;
	};

}