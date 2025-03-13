#include "SourceEq.hpp"

opt::FunctorSourceEq::FunctorSourceEq(SourceFunctor functor,
	SourceTrigger trigger)
	: m_functor(std::move(functor)), m_trigger{std::move(trigger)}
{
}

void opt::FunctorSourceEq::Apply(std::span<double> output, const double& t, SourceEvent& ev)
{
	m_functor(output, t, ev);
}

void opt::FunctorSourceEq::EventTrigger(const double& t, SourceEvent& ev)
{
	if (m_trigger)
	{
		m_trigger(t, ev);
	}
}

