#pragma once
#include <functional>
#include <span>
#include <memory_resource>
#include "optimizer/Equation.hpp"
#include <any>

namespace opt
{
class Observer : public InputEquation
{
public:
	using ObserverFunctor = std::function<void(
		std::span<const double>, const double&)>;
	using InitializeFunctor = std::function<void(const double, const double)>;
	using CaptureEndFunctor = std::function<void(const double)>;
	using GetResultsFunctor = std::function<std::any()>;

	Observer(std::pmr::vector<int32_t> input_ids,
		ObserverFunctor functor,
		InitializeFunctor init_functor = {},
		CaptureEndFunctor end_functor = {},
		GetResultsFunctor results_functor = {},
		std::pmr::memory_resource* resource = std::pmr::get_default_resource());
	void Apply(const double& t);
	void Initialize(const double t_begin, const double t_end);
	void CaptureEnd(const double t_end);
	std::any GetResults();
private:
	ObserverFunctor m_functor;
	InitializeFunctor m_init_functor;
	CaptureEndFunctor m_end_functor;
	GetResultsFunctor m_results_functor;
};
}