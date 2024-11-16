#include "SourceEq.hpp"

opt::SourceEq::SourceEq(std::pmr::vector<int32_t> output_ids, SourceFunctor functor,
	SourceTrigger trigger, std::pmr::memory_resource* resource)
	:OutputEquation(std::move(output_ids), resource), m_functor(std::move(functor)), m_trigger{std::move(trigger)}
{
}

void opt::SourceEq::Apply(const double& t)
{
	m_functor(m_output_buffer, t, *this);
}

void opt::SourceEq::EventTrigger(const double& t)
{
	if (m_trigger)
	{
		m_trigger(t, *this);
	}
}

