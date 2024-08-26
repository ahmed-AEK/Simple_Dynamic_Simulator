#pragma once
#include "optimizer/FatAny.hpp"
#include <functional>
#include <span>
#include <memory_resource>
#include "optimizer/Equation.hpp"

namespace opt
{
class SourceEq : public Equation
{
public:
	using SourceFunctor = std::function<void(
		std::span<double>, const double&)>;

	SourceEq(std::pmr::vector<int64_t> output_ids,
		SourceFunctor functor,
		std::pmr::memory_resource* resource = std::pmr::get_default_resource());
	void Apply(const double& t);
private:
	SourceFunctor m_functor;
};
}