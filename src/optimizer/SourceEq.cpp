#pragma once
#include "SourceEq.hpp"

opt::SourceEq::SourceEq(std::pmr::vector<int64_t> output_ids, SourceFunctor functor, std::pmr::memory_resource* resource)
	:Equation({}, std::move(output_ids), resource), m_functor(std::move(functor))
{
}

void opt::SourceEq::Apply(const double& t)
{
	m_functor(m_output_buffer, t);
}

