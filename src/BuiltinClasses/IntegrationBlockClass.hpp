#pragma once

#include "PluginAPI/BlockClass.hpp"
#include "BuiltinClasses/BuiltinBasicClass.hpp"

namespace node
{

class IntegrationBlockClass : public BuiltinBasicClass
{
public:
	IntegrationBlockClass();
	GetFunctorResult GetFunctor(const std::vector<model::BlockProperty>& properties) const override;
};

}
