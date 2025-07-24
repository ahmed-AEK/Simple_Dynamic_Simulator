#pragma once

#include "PluginAPI/BlockClass.hpp"
#include "BuiltinClasses/BuiltinBasicClass.hpp"

namespace node
{

class ElectricalControlledVDCBlockClass : public BuiltinBasicClass
{
public:
	ElectricalControlledVDCBlockClass();
	int GetFunctor(std::span<const model::BlockProperty> properties, IGetFunctorCallback& cb) const override;
	void CalculateSockets(std::span<const model::BlockProperty> properties, ICalculateSocketCallback& cb) const override;
};

}
