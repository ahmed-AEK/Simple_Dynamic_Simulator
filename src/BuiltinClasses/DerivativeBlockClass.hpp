#pragma once

#include "PluginAPI/BlockClass.hpp"
#include "BuiltinClasses/BuiltinBasicClass.hpp"

namespace node
{

class DerivativeBlockClass : public BuiltinBasicClass
{
public:
	DerivativeBlockClass();
	int GetFunctor(std::span<const model::BlockProperty> properties, IGetFunctorCallback& cb) const override;
};

}
