#include "Observer.hpp"

opt::FunctorObserver::FunctorObserver(
	ObserverFunctor functor,
	InitializeFunctor init_functor,
	CaptureEndFunctor end_functor, 
	GetResultsFunctor results_functor)
	: m_functor(std::move(functor)), 
	m_init_functor{std::move(init_functor)}, m_end_functor{std::move(end_functor)},
	m_results_functor{std::move(results_functor)}
{
}

void opt::FunctorObserver::Apply(std::span<const double> input, const double t)
{
	m_functor(input, t);
}

void opt::FunctorObserver::Initialize(const double t_begin, const double t_end)
{
	if (m_init_functor)
	{
		m_init_functor(t_begin, t_end);
	}
}

void opt::FunctorObserver::CaptureEnd(const double t_end)
{
	if (m_end_functor)
	{
		m_end_functor(t_end);
	}
}

std::any opt::FunctorObserver::GetResults()
{
	assert(m_results_functor);
	if (m_results_functor)
	{
		return m_results_functor();
	}
	return std::any{};
}
