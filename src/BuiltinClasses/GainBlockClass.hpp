#pragma once

#include "PluginAPI/BlockClass.hpp"
#include "BuiltinClasses/BuiltinBasicClass.hpp"

namespace node
{

class GainBlockClass : public BuiltinBasicClass
{
public:
	GainBlockClass();
	int GetFunctor(std::span<const model::BlockProperty> properties, IGetFunctorCallback& cb) const override;
};

}
