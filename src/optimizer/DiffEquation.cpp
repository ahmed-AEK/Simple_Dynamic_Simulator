#include "DiffEquation.hpp"
#include <cassert>





opt::DiffEquation::DiffEquation(std::pmr::vector<int32_t> input_ids,
	std::pmr::vector<int32_t> output_ids, DiffFunctor functor,
	std::pmr::memory_resource* resource)
	:InputEquation{ std::move(input_ids), resource }, OutputEquation{ std::move(output_ids), resource }, m_functor{ std::move(functor) }
{
}

void opt::DiffEquation::Apply(const double t)
{
	assert(m_input_buffer.size() == m_input_ids.size());
	assert(m_output_buffer.size() == m_output_ids.size());
	m_functor(m_input_buffer, m_output_buffer, t);
}
