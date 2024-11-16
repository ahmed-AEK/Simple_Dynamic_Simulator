#pragma once

#include <vector>
#include <memory_resource>
#include <functional>
#include <span>
#include "optimizer/Equation.hpp"

namespace opt
{
class NLEquation : public InputEquation, public OutputEquation
{
public:
	using NLFunctor = std::function<void(std::span<const double>, std::span<double>)>;

	NLEquation(std::pmr::vector<int32_t> input_ids, 
		std::pmr::vector<int32_t> output_ids, 
		NLFunctor functor,
		std::pmr::memory_resource* resource = std::pmr::get_default_resource());
	void Apply();
private:
	NLFunctor m_functor;
};
}