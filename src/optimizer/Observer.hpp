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
	using InitializeFunctor = std::function<void(const double, const double)>;
	using CaptureEndFunctor = std::function<void(const double)>;

	Observer(std::pmr::vector<int64_t> input_ids,
		ObserverFunctor functor,
		InitializeFunctor init_functor = {},
		CaptureEndFunctor end_functor = {},
		std::pmr::memory_resource* resource = std::pmr::get_default_resource());
	void Apply(const double& t);
	void Initialize(const double t_begin, const double t_end);
	void CaptureEnd(const double t_end);
private:
	ObserverFunctor m_functor;
	InitializeFunctor m_init_functor;
	CaptureEndFunctor m_end_functor;
};
}