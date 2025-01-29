#include "Observer.hpp"

opt::Observer::Observer(std::pmr::vector<int32_t> input_ids, 
	ObserverFunctor functor,
	InitializeFunctor init_functor,
	CaptureEndFunctor end_functor, 
	GetResultsFunctor results_functor,
	std::pmr::memory_resource* resource)
	:InputEquation(std::move(input_ids), resource), m_functor(std::move(functor)), 
	m_init_functor{std::move(init_functor)}, m_end_functor{std::move(end_functor)},
	m_results_functor{std::move(results_functor)}
{
}

void opt::Observer::Apply(const double& t)
{
	m_functor(m_input_buffer, t);
}

void opt::Observer::Initialize(const double t_begin, const double t_end)
{
	if (m_init_functor)
	{
		m_init_functor(t_begin, t_end);
	}
}

void opt::Observer::CaptureEnd(const double t_end)
{
	if (m_end_functor)
	{
		m_end_functor(t_end);
	}
}

std::any opt::Observer::GetResults()
{
	assert(m_results_functor);
	if (m_results_functor)
	{
		return m_results_functor();
	}
	return std::any{};
}
