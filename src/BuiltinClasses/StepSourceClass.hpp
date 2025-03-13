#pragma once

#include "PluginAPI/BlockClass.hpp"
#include "BuiltinClasses/BuiltinBasicClass.hpp"

namespace node
{

	class StepSourceClass : public BuiltinBasicClass
	{
	public:
		StepSourceClass();
		GetFunctorResult GetFunctor(const std::vector<model::BlockProperty>& properties) const override;
	};

}