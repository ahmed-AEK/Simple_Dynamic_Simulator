#include "NLStatefulEquation.hpp"

opt::NLStatefulEquation::NLStatefulEquation(std::pmr::vector<int64_t> input_ids, std::pmr::vector<int64_t> output_ids, NLStatefulFunctor functor, std::pmr::memory_resource* resource)
	:Equation(std::move(input_ids), std::move(output_ids), resource), m_functor(std::move(functor))
{
}

opt::FatAny opt::NLStatefulEquation::Apply(const double& t, const FatAny& old_state)
{
	return m_functor(m_input_buffer, m_output_buffer, t, old_state);
}
