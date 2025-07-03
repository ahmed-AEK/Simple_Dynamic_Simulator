#include "FlowEquation.hpp"

opt::FunctorFlowEquation::FunctorFlowEquation(FlowFunctor functor)
	:m_functor{ std::move(functor) }
{
}

opt::Status opt::FunctorFlowEquation::Apply(std::span<const double> input, std::span<double> output)
{
	m_functor(input, output);
	return Status::ok;
}
