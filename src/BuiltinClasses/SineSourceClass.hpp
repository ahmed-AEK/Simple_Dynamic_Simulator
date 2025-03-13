#pragma once

#include "PluginAPI/BlockClass.hpp"
#include "BuiltinClasses/BuiltinBasicClass.hpp"

namespace node
{

class SineSourceClass : public BuiltinBasicClass
{
public:
	SineSourceClass();
	GetFunctorResult GetFunctor(const std::vector<model::BlockProperty>& properties) const override;
};

}