#include "NLStatefulEquation.hpp"

opt::NLStatefulEquation::NLStatefulEquation(std::pmr::vector<int32_t> input_ids, std::pmr::vector<int32_t> output_ids, 
	NLStatefulFunctor functor, NLStatefulUpdateFunctor update_functor,
	NLStatefulCrossFunctor cross_functor,
	NLStatefulEventFunctor event_functor,
	std::pmr::memory_resource* resource)
	:InputEquation{ std::move(input_ids), resource },
	OutputEquation{ std::move(output_ids), resource }, 
	m_functor{std::move(functor)}, m_update_functor{ std::move(update_functor) },
	m_cross_functor{std::move(cross_functor)}, m_event_functor{std::move(event_functor)}
{
}

void opt::NLStatefulEquation::Apply(const double& t)
{
	m_functor(m_input_buffer, m_output_buffer, t, *this);
}

void opt::NLStatefulEquation::Update(const double& t)
{
	m_update_functor(m_input_buffer, t, *this);
}

void opt::NLStatefulEquation::CrossTrigger(const double& t, size_t index)
{
	assert(m_cross_functor);
	if (m_cross_functor)
	{
		m_cross_functor(t, index, *this);
	}
}

void opt::NLStatefulEquation::EventTrigger(const double& t)
{
	assert(m_event_functor);
	if (m_event_functor)
	{
		m_event_functor(t, *this);
	}
}
