#pragma once
#include "optimizer/FatAny.hpp"
#include "optimizer/Equation.hpp"

#include <functional>
#include <span>
#include <memory_resource>
#include <optional>
#include <memory>

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

	double value{};
	int16_t in_port_id{};
	CrossType type{};
	Position last_value{};
	Position current_value{};
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
	[[nodiscard]] virtual Status Apply(std::span<const double> input, std::span<double> output, double t, NLStatefulEquationDataCRef data) = 0;
	[[nodiscard]] virtual Status Update(std::span<const double> input, double t, NLStatefulEquationDataRef data) = 0;
	[[nodiscard]] virtual Status CrossTrigger(double t, size_t index, NLStatefulEquationDataRef data) {
		UNUSED_PARAM(t);
		UNUSED_PARAM(index);
		UNUSED_PARAM(data);
		return Status::ok;
	}
	[[nodiscard]] virtual Status EventTrigger(double t, NLStatefulEquationDataRef data)
	{
		UNUSED_PARAM(t);
		UNUSED_PARAM(data);
		return Status::ok;
	}
	virtual void Destroy() { delete this; }
	virtual const char* GetLastError() { return "unknown error"; }
	virtual void ClearError() {}

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

	Status Apply(std::span<const double> input, std::span<double> output, double t, NLStatefulEquationDataCRef data) override;
	Status Update(std::span<const double> input, double t, NLStatefulEquationDataRef data) override;
	Status CrossTrigger(double t, size_t index, NLStatefulEquationDataRef data) override;
	Status EventTrigger(double t, NLStatefulEquationDataRef data) override;
private:
	NLStatefulFunctor m_functor;
	NLStatefulUpdateFunctor m_update_functor;
	NLStatefulCrossFunctor m_cross_functor;
	NLStatefulEventFunctor m_event_functor;
};

struct NLStatefulEquationWrapper;

struct NLStatefulEquationView
{
	std::span<const int32_t> input_ids;
	std::span<const int32_t> output_ids;
	NLStatefulEqPtr& equation;
	std::span<const ZeroCrossDescriptor> crossings;
	StatefulEquationEvent& ev;

	NLStatefulEquationWrapper ToFunctor() &&;
};

struct NLStatefulEquationWrapper
{
	std::vector<int32_t> input_ids;
	std::vector<int32_t> output_ids;
	NLStatefulEqPtr equation;
	NLStatefulEquationData data;

	operator NLStatefulEquationView()
	{
		return NLStatefulEquationView{
			input_ids, output_ids, equation, data.crossings, data.ev
		};
	}
};

inline NLStatefulEquationWrapper NLStatefulEquationView::ToFunctor() &&
{
	return {
	{input_ids.begin(), input_ids.end()},
	{output_ids.begin(), output_ids.end()},
	std::move(equation),
	{{crossings.begin(), crossings.end()}, ev}
	};
}
}


