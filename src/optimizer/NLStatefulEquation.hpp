#pragma once
#include "optimizer/FatAny.hpp"
#include <functional>
#include <span>
#include <memory_resource>
#include "optimizer/Equation.hpp"
#include <optional>

namespace opt
{

struct StatefulEquationEvent
{
	bool enabled = false;
	bool set = true;
	double t = 0;
};

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

struct NLStatefulEquationDataCRef
{
	std::span<const ZeroCrossDescriptor> crossings;
	const StatefulEquationEvent& ev;
};

struct NLStatefulEquationDataRef
{
	std::span<ZeroCrossDescriptor> crossings;
	StatefulEquationEvent& ev;

	operator NLStatefulEquationDataCRef() const
	{
		return { crossings, ev };
	}
};

class INLStatefulEquation
{
public:
	virtual void Apply(std::span<const double> input, std::span<double> output, double t, NLStatefulEquationDataCRef data) = 0;
	virtual void Update(std::span<const double> input, double t, NLStatefulEquationDataRef data) = 0;
	virtual void CrossTrigger(double t, size_t index, NLStatefulEquationDataRef data) {
		UNUSED_PARAM(t);
		UNUSED_PARAM(index);
		UNUSED_PARAM(data);
	}
	virtual void EventTrigger(double t, NLStatefulEquationDataRef data)
	{
		UNUSED_PARAM(t);
		UNUSED_PARAM(data);
	}
	virtual void Destroy() { delete this; }
protected:
	virtual ~INLStatefulEquation() = default;
};

namespace detail
{
	struct NLStatefulEqDeleter
	{
		void operator()(INLStatefulEquation* ptr) { if (ptr) { ptr->Destroy(); } };
	};
}

using NLStatefulEqPtr = std::unique_ptr<INLStatefulEquation, detail::NLStatefulEqDeleter>;

template <typename T, typename...Args>
NLStatefulEqPtr make_NLStatefulEqn(Args&&...args)
{
	return NLStatefulEqPtr{ new T{std::forward<Args>(args)...} };
}

struct NLStatefulEquationData
{
	std::vector<ZeroCrossDescriptor> crossings;
	StatefulEquationEvent ev;

	operator NLStatefulEquationDataRef()
	{
		return { crossings, ev };
	}
	
	operator NLStatefulEquationDataCRef() const
	{
		return { crossings, ev };
	}
};

class FunctorNLStatefulEquation : public INLStatefulEquation
{
public:
	using NLStatefulFunctor = std::function<void(
		std::span<const double>, std::span<double>, double, NLStatefulEquationDataCRef)>;
	using NLStatefulUpdateFunctor = std::function<void(
		std::span<const double>, double, NLStatefulEquationDataRef)>;
	using NLStatefulCrossFunctor = std::function<void(double, size_t, NLStatefulEquationDataRef)>;
	using NLStatefulEventFunctor = std::function<void(double, NLStatefulEquationDataRef)>;

	FunctorNLStatefulEquation(
		NLStatefulFunctor functor,
		NLStatefulUpdateFunctor update_functor,
		NLStatefulCrossFunctor cross_functor = {},
		NLStatefulEventFunctor event_functor = {});

	void Apply(std::span<const double> input, std::span<double> output, double t, NLStatefulEquationDataCRef data) override;
	void Update(std::span<const double> input, double t, NLStatefulEquationDataRef data) override;
	void CrossTrigger(double t, size_t index, NLStatefulEquationDataRef data) override;
	void EventTrigger(double t, NLStatefulEquationDataRef data) override;
private:
	NLStatefulFunctor m_functor;
	NLStatefulUpdateFunctor m_update_functor;
	NLStatefulCrossFunctor m_cross_functor;
	NLStatefulEventFunctor m_event_functor;
};

struct NLStatefulEquationWrapper
{
	std::vector<int32_t> input_ids;
	std::vector<int32_t> output_ids;
	NLStatefulEqPtr equation;
	NLStatefulEquationData data;
};
}


