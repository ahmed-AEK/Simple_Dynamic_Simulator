#include "NLEquation.hpp"
#include <cassert>





opt::NLEquation::NLEquation(std::pmr::vector<int64_t> input_ids, 
	std::pmr::vector<int64_t> output_ids, NLFunctor functor,
	std::pmr::memory_resource* resource)
	:Equation(std::move(input_ids), std::move(output_ids), resource)
	, m_functor(std::move(functor))
{
}

void opt::NLEquation::Apply()
{
	assert(m_input_buffer.size() == m_input_ids.size());
	assert(m_output_buffer.size() == m_output_ids.size());
	m_functor(m_input_buffer, m_output_buffer);
}
