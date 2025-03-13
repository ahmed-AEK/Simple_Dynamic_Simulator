#pragma once

#include "PluginAPI/BlockClass.hpp"
#include "BuiltinClasses/BuiltinBasicClass.hpp"

namespace node
{

class ConstantSourceClass : public BuiltinBasicClass
{
public:
	ConstantSourceClass();
	GetFunctorResult GetFunctor(const std::vector<model::BlockProperty>& properties) const override;
};

}