#pragma once

#include "PluginAPI/BlockClass.hpp"
#include "BuiltinClasses/BuiltinBasicClass.hpp"

namespace node
{

class DerivativeBlockClass : public BuiltinBasicClass
{
public:
	DerivativeBlockClass();
	GetFunctorResult GetFunctor(const std::vector<model::BlockProperty>& properties) const override;
};

}
