#pragma once
#include "optimizer/FatAny.hpp"
#include <functional>
#include <span>
#include <memory_resource>
#include "optimizer/Equation.hpp"
#include <optional>

namespace opt
{

struct ZeroCrossDescriptor
{
	enum class CrossType : char
	{
		rising,
		falling,
		both,
	};
	enum class Position : char
	{
		undefined,
		above,
		below,
	};

	double value;
	int16_t in_port_id;
	CrossType type;
	Position last_value;
	Position current_value;

};

class NLStatefulEquation : public InputEquation, public OutputEquation
{
public:
	struct EquationEvent
	{
		double t = 0;
		bool set = true;
	};
	using NLStatefulFunctor = std::function<void(
		std::span<const double>, std::span<double>, const double&, const NLStatefulEquation&)>;
	using NLStatefulUpdateFunctor = std::function<void(
		std::span<const double>, const double&, NLStatefulEquation&)>;
	using NLStatefulCrossFunctor = std::function<void(const double&, size_t index, NLStatefulEquation&)>;
	using NLStatefulEventFunctor = std::function<void(const double&, NLStatefulEquation&)>;

	NLStatefulEquation(std::pmr::vector<int32_t> input_ids,
		std::pmr::vector<int32_t> output_ids,
		NLStatefulFunctor functor,
		NLStatefulUpdateFunctor update_functor,
		NLStatefulCrossFunctor cross_functor = {},
		NLStatefulEventFunctor event_functor = {},
		std::pmr::memory_resource* resource = std::pmr::get_default_resource());

	void Apply(const double& t);
	void Update(const double& t);
	void CrossTrigger(const double& t, size_t index);

	const FatAny& GetState() const { return m_state; }
	FatAny& GetState() { return m_state; }

	const std::vector<ZeroCrossDescriptor>& GetZeroCrossings() const { return m_crossings; }
	std::vector<ZeroCrossDescriptor>& GetZeroCrossings() { return m_crossings; }

	void EventTrigger(const double& t);
	std::optional<EquationEvent>& GetEvent() { return m_event; }
	const std::optional<EquationEvent>& GetEvent() const { return m_event; }

private:
	NLStatefulFunctor m_functor;
	NLStatefulUpdateFunctor m_update_functor;
	NLStatefulCrossFunctor m_cross_functor;
	NLStatefulEventFunctor m_event_functor;
	std::vector<ZeroCrossDescriptor> m_crossings;
	FatAny m_state;
	std::optional<EquationEvent> m_event;
};
}