#include "PotentialEquation.hpp"

opt::FunctorPotentialEquation::FunctorPotentialEquation(PotentialFunctor functor)
	:m_functor{ std::move(functor) }
{
}

opt::Status opt::FunctorPotentialEquation::Apply(std::span<const double> input, double flow, double& potential)
{
	m_functor(input, flow, potential);
	return Status::ok;
}
