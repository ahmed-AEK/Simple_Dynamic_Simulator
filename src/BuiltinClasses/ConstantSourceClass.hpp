#pragma once

#include "PluginAPI/BlockClass.hpp"
#include "BuiltinClasses/BuiltinBasicClass.hpp"

namespace node
{

class ConstantSourceClass : public BuiltinBasicClass
{
public:
	ConstantSourceClass();
	int GetFunctor(std::span<const model::BlockProperty> properties, IGetFunctorCallback& cb) const override;
};

}