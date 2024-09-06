#pragma once
#include <vector>
#include <memory_resource>
#include <functional>
#include <span>
#include "optimizer/Equation.hpp"

namespace opt
{
class DiffEquation : public Equation
{
public:
	using DiffFunctor = std::function<void(std::span<const double>, std::span<double>, const double)>;

	DiffEquation(std::pmr::vector<int64_t> input_ids,
		std::pmr::vector<int64_t> output_ids, 
		DiffFunctor functor,
		std::pmr::memory_resource* resource = std::pmr::get_default_resource());
	void Apply(const double t);
private:
	DiffFunctor m_functor;
};
}