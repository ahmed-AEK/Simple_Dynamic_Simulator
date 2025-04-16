#pragma once

#include "PluginAPI/BlockClass.hpp"
#include "BuiltinClasses/BuiltinBasicClass.hpp"

namespace node
{

class ComparatorBlockClass : public BuiltinBasicClass
{
public:
	ComparatorBlockClass();
	int GetFunctor(std::span<const model::BlockProperty> properties, IGetFunctorCallback& cb) const override;
};

}
