#include "NLStatefulEquation.hpp"

opt::FunctorNLStatefulEquation::FunctorNLStatefulEquation(
	NLStatefulFunctor functor, NLStatefulUpdateFunctor update_functor,
	NLStatefulCrossFunctor cross_functor,
	NLStatefulEventFunctor event_functor)
	: m_functor{std::move(functor)}, m_update_functor{ std::move(update_functor) },
	m_cross_functor{std::move(cross_functor)}, m_event_functor{std::move(event_functor)}
{
}

void opt::FunctorNLStatefulEquation::Apply(std::span<const double> input, std::span<double> output, double t, NLStatefulEquationDataCRef data)
{
	m_functor(input, output, t, data);
}

void opt::FunctorNLStatefulEquation::Update(std::span<const double> input, double t, NLStatefulEquationDataRef data)
{
	m_update_functor(input, t, data);
}

void opt::FunctorNLStatefulEquation::CrossTrigger(double t, size_t index, NLStatefulEquationDataRef data)
{
	assert(m_cross_functor);
	if (m_cross_functor)
	{
		m_cross_functor(t, index, data);
	}
}

void opt::FunctorNLStatefulEquation::EventTrigger(double t, NLStatefulEquationDataRef data)
{
	assert(m_event_functor);
	if (m_event_functor)
	{
		m_event_functor(t, data);
	}
}
