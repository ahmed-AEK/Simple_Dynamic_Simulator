#include "Observer.hpp"

opt::Observer::Observer(std::pmr::vector<int64_t> input_ids, ObserverFunctor functor, std::pmr::memory_resource* resource)
	:Equation(std::move(input_ids), {}, resource), m_functor(std::move(functor))
{
}

void opt::Observer::Apply(const double& t)
{
	m_functor(m_input_buffer, t);
}
