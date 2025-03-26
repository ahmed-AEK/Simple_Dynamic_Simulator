#include "DiffEquation.hpp"
#include <cassert>

opt::FunctorDiffEquation::FunctorDiffEquation(opt::FunctorDiffEquation::DiffFunctor functor)
	: m_functor{std::move(functor)}
{
}

opt::Status opt::FunctorDiffEquation::Apply(std::span<const double> input, std::span<double> output, const double t)
{
	m_functor(input, output, t);
	return Status::ok;
}
