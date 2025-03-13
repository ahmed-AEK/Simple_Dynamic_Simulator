#include "NLEquation.hpp"

opt::FunctorNLEquation::FunctorNLEquation(NLFunctor functor)
	:m_functor{std::move(functor)}
{
}

void opt::FunctorNLEquation::Apply(std::span<const double> input, std::span<double> output)
{
	m_functor(input, output);
}
