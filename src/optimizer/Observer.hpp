#pragma once
#include "optimizer/FatAny.hpp"
#include <functional>
#include <span>
#include <memory_resource>
#include "optimizer/Equation.hpp"

namespace opt
{
class Observer : public Equation
{
public:
	using ObserverFunctor = std::function<void(
		std::span<const double>, const double&)>;

	Observer(std::pmr::vector<int64_t> input_ids,
		ObserverFunctor functor,
		std::pmr::memory_resource* resource = std::pmr::get_default_resource());
	void Apply(const double& t);
private:
	ObserverFunctor m_functor;
};
}