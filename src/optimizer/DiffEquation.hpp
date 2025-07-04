#pragma once
#include <vector>
#include <memory_resource>
#include <functional>
#include <span>
#include "optimizer/Equation.hpp"

namespace opt
{

class IDiffEquation
{
public:
	[[nodiscard]] virtual Status Apply(std::span<const double> input, std::span<double> output, const double t) = 0;
	virtual void Destroy() { delete this; }
	virtual const char* GetLastError() { return "unknown error"; }
	virtual void ClearError() {}
protected:
	virtual ~IDiffEquation() = default;
};

namespace detail
{
	struct DiffEqnDeleter
	{
		void operator()(IDiffEquation* ptr) { if (ptr) { ptr->Destroy(); } };
	};
}

using DiffEqPtr = std::unique_ptr<IDiffEquation, detail::DiffEqnDeleter>;

template <typename T, typename...Args>
DiffEqPtr make_DiffEqn(Args&&...args)
{
	return DiffEqPtr{ new T{std::forward<Args>(args)...} };
}

class FunctorDiffEquation : public IDiffEquation
{
public:
	using DiffFunctor = std::function<void(std::span<const double> input, std::span<double> output, const double t)>;

	explicit FunctorDiffEquation(DiffFunctor functor);
	Status Apply(std::span<const double> input, std::span<double> output, const double t) override;
private:
	DiffFunctor m_functor;
};

struct DiffEquationWrapper;

struct DiffEquationView
{
	std::span<const int32_t> input_ids;
	std::span<const int32_t> output_ids;
	DiffEqPtr& equation;

	DiffEquationWrapper ToFunctor() &&;
};

struct DiffEquationWrapper
{
	std::vector<int32_t> input_ids;
	std::vector<int32_t> output_ids;
	DiffEqPtr equation;

	operator DiffEquationView()
	{
		return  DiffEquationView{ input_ids, output_ids, equation };
	}
};

inline DiffEquationWrapper DiffEquationView::ToFunctor() &&
{
	return {
		{input_ids.begin(), input_ids.end()},
		{output_ids.begin(), output_ids.end()},
		std::move(equation)
	};
}

}