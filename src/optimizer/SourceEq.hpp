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

struct SourceEvent
{
	bool enabled = false;
	bool set = false;
	double t = 0;
};

class ISourceEq
{
public:
	[[nodiscard]] virtual Status Apply(std::span<double> output, const double& t, SourceEvent& ev) = 0;
	[[nodiscard]] virtual Status EventTrigger(const double& t, SourceEvent& ev) = 0;
	virtual void Destroy() { delete this; }
	virtual const char* GetLastError() { return "unknown error"; }
	virtual void ClearError() {}
protected:
	virtual ~ISourceEq() = default;
};

namespace detail
{
	struct SourceEqnDeleter
	{
		void operator()(ISourceEq* ptr) { if (ptr) { ptr->Destroy(); } };
	};
}

using SourceEqPtr = std::unique_ptr<ISourceEq, detail::SourceEqnDeleter>;

template <typename T, typename...Args>
SourceEqPtr make_SourceEqn(Args&&...args)
{
	return SourceEqPtr{ new T{std::forward<Args>(args)...} };
}


class FunctorSourceEq : public ISourceEq
{
public:
	using SourceFunctor = std::function<void(
		std::span<double> out, const double& t, SourceEvent& ev)>;

	using SourceTrigger = std::function<void(const double&, SourceEvent&)>;

	explicit FunctorSourceEq(
		SourceFunctor functor,
		SourceTrigger trigger = {});
	Status Apply(std::span<double> output, const double& t, SourceEvent& ev) override;

	Status EventTrigger(const double& t, SourceEvent& ev) override;
	FunctorSourceEq(FunctorSourceEq&&) = delete;
	FunctorSourceEq& operator=(FunctorSourceEq&&) = delete;
private:
	SourceFunctor m_functor;
	SourceTrigger m_trigger;
	opt::FatAny m_state;
};

struct SourceEqWrapper;

struct SourceEqView
{
	std::span<const int32_t> output_ids;
	SourceEqPtr& equation;
	SourceEvent& ev;

	SourceEqWrapper ToFunctor() &&;
};

struct SourceEqWrapper
{
	std::vector<int32_t> output_ids;
	SourceEqPtr equation;
	SourceEvent ev;

	operator SourceEqView()
	{
		return SourceEqView{ output_ids, equation, ev };
	}
	static SourceEqWrapper FromView(SourceEqView&& view)
	{
		return {
			{view.output_ids.begin(), view.output_ids.end()},
			std::move(view.equation),
			view.ev
		};
	}
};

inline SourceEqWrapper SourceEqView::ToFunctor() &&
{
	return {
	{output_ids.begin(), output_ids.end()},
	std::move(equation),
	ev
	};
}
}