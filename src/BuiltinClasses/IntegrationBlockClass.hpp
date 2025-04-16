#pragma once

#include "PluginAPI/BlockClass.hpp"
#include "BuiltinClasses/BuiltinBasicClass.hpp"

namespace node
{

class IntegrationBlockClass : public BuiltinBasicClass
{
public:
	IntegrationBlockClass();
	int GetFunctor(std::span<const model::BlockProperty> properties, IGetFunctorCallback& cb) const override;
};

}
