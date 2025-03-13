#pragma once
#include "optimizer/FatAny.hpp"
#include <functional>
#include <span>
#include <memory_resource>
#include "optimizer/Equation.hpp"
#include <optional>

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
	virtual void Apply(std::span<double> output, const double& t, SourceEvent& ev) = 0;
	virtual void EventTrigger(const double& t, SourceEvent& ev) = 0;
	virtual void Destroy() { delete this; }
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
		std::span<double>, const double&, SourceEvent&)>;

	using SourceTrigger = std::function<void(const double&, SourceEvent&)>;

	FunctorSourceEq(
		SourceFunctor functor,
		SourceTrigger trigger = {});
	void Apply(std::span<double> output, const double& t, SourceEvent& ev);

	void EventTrigger(const double& t, SourceEvent& ev);

private:
	SourceFunctor m_functor;
	SourceTrigger m_trigger;
	opt::FatAny m_state;
};

struct SourceEqWrapper
{
	std::vector<int32_t> output_ids;
	SourceEqPtr equation;
	SourceEvent ev;
};
}