#pragma once

#include "PluginAPI/BlockClass.hpp"
#include "BuiltinClasses/BuiltinBasicClass.hpp"

namespace node
{

class AddSimpleBlockClass : public BuiltinBasicClass
{
public:
	AddSimpleBlockClass();
	GetFunctorResult GetFunctor(const std::vector<model::BlockProperty>& properties) const override;
};

}
