#pragma once

#include "optimizer/Equation.hpp"

#include <functional>
#include <span>
#include <memory_resource>
#include <any>
#include <memory>

namespace opt
{

class IObserver
{
public:
	[[nodiscard]] virtual Status Apply(std::span<const double> input, const double t) = 0;
	[[nodiscard]] virtual Status Initialize(const double t_begin, const double t_end) = 0;
	[[nodiscard]] virtual Status CaptureEnd(const double t_end) = 0;
	virtual std::any GetResults() = 0;
	virtual void Destroy() { delete this; }
	virtual const char* GetLastError() { return "unknown error"; }
	virtual void ClearError() {}
protected:
	virtual ~IObserver() = default;
};


namespace detail
{
	struct ObsEqnDeleter
	{
		void operator()(IObserver* ptr) { if (ptr) { ptr->Destroy(); } };
	};
}

using ObserverEqPtr = std::unique_ptr<IObserver, detail::ObsEqnDeleter>;

template <typename T, typename...Args>
ObserverEqPtr make_ObserverEqn(Args&&...args)
{
	return ObserverEqPtr{ new T{std::forward<Args>(args)...} };
}

class FunctorObserver : public IObserver
{
public:
	using ObserverFunctor = std::function<void(
		std::span<const double>, const double)>;
	using InitializeFunctor = std::function<void(const double, const double)>;
	using CaptureEndFunctor = std::function<void(const double)>;
	using GetResultsFunctor = std::function<std::any()>;

	explicit FunctorObserver(
		ObserverFunctor functor,
		InitializeFunctor init_functor = {},
		CaptureEndFunctor end_functor = {},
		GetResultsFunctor results_functor = {}
	);
	[[nodiscard]] Status Apply(std::span<const double> input, const double t) override;
	[[nodiscard]] Status Initialize(const double t_begin, const double t_end) override;
	[[nodiscard]] Status CaptureEnd(const double t_end) override;
	std::any GetResults() override;
private:
	ObserverFunctor m_functor;
	InitializeFunctor m_init_functor;
	CaptureEndFunctor m_end_functor;
	GetResultsFunctor m_results_functor;
};

struct ObserverWrapper;

struct ObserverView
{
	std::span<const int32_t> input_ids;
	ObserverEqPtr& equation;

	ObserverWrapper ToFunctor() &&;
};

struct ObserverWrapper
{
	std::vector<int32_t> input_ids;
	ObserverEqPtr equation;

	operator ObserverView()
	{
		return ObserverView{ input_ids, equation };
	}
};

inline ObserverWrapper ObserverView::ToFunctor() &&
{
	return {
	{input_ids.begin(), input_ids.end()},
	std::move(equation)
	};
}
}