#pragma once
#include "optimizer/FatAny.hpp"
#include <functional>
#include <span>
#include <memory_resource>
#include "optimizer/Equation.hpp"
#include <optional>

namespace opt
{
class SourceEq : public Equation
{
public:
	struct SourceEvent
	{
		double t = 0;
		bool set = false;
	};

	using SourceFunctor = std::function<void(
		std::span<double>, const double&, SourceEq&)>;

	SourceEq(std::pmr::vector<int64_t> output_ids,
		SourceFunctor functor,
		std::pmr::memory_resource* resource = std::pmr::get_default_resource());
	void Apply(const double& t);
	std::optional<SourceEvent>& GetEvent() { return m_event; }
private:
	SourceFunctor m_functor;
	std::optional<SourceEvent> m_event;
};
}