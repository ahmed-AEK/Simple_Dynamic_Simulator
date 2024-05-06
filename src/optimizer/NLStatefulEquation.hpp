#pragma once
#include "optimizer/FatAny.hpp"
#include <functional>
#include <span>
#include <memory_resource>
#include "optimizer/Equation.hpp"

namespace opt
{
class NLStatefulEquation : public Equation
{
public:
	using NLStatefulFunctor = std::function<FatAny(
		std::span<const double>, std::span<double>, const double&, const FatAny&)>;

	NLStatefulEquation(std::pmr::vector<int64_t> input_ids,
		std::pmr::vector<int64_t> output_ids,
		NLStatefulFunctor functor,
		std::pmr::memory_resource* resource = std::pmr::get_default_resource());
	[[nodiscard]] opt::FatAny Apply(const double& t, const FatAny& old_state);
private:
	NLStatefulFunctor m_functor;
};
}