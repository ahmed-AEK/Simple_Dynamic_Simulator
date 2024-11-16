#pragma once
#include "optimizer/FatAny.hpp"
#include <functional>
#include <span>
#include <memory_resource>
#include "optimizer/Equation.hpp"
#include <optional>

namespace opt
{
class SourceEq : public OutputEquation
{
public:
	struct SourceEvent
	{
		double t = 0;
		bool set = false;
	};

	using SourceFunctor = std::function<void(
		std::span<double>, const double&, SourceEq&)>;

	using SourceTrigger = std::function<void(const double&, SourceEq&)>;

	SourceEq(std::pmr::vector<int32_t> output_ids,
		SourceFunctor functor,
		SourceTrigger trigger = {},
		std::pmr::memory_resource* resource = std::pmr::get_default_resource());
	void Apply(const double& t);

	void EventTrigger(const double& t);
	std::optional<SourceEvent>& GetEvent() { return m_event; }
	const std::optional<SourceEvent>& GetEvent() const { return m_event; }
	const opt::FatAny& GetState() const { return m_state; };
	opt::FatAny& GetState() { return m_state; }

private:
	SourceFunctor m_functor;
	SourceTrigger m_trigger;
	std::optional<SourceEvent> m_event;
	opt::FatAny m_state;
};
}