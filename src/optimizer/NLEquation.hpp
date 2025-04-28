#pragma once

#include "optimizer/Equation.hpp"

#include <memory>
#include <vector>
#include <memory_resource>
#include <functional>
#include <span>

namespace opt
{

class INLEquation
{
public:
	[[nodiscard]] virtual Status Apply(std::span<const double> input, std::span<double> output) = 0;
	virtual void Destroy() { delete this; }
	virtual const char* GetLastError() { return "unknown error"; }
	virtual void ClearError() {}
protected:
	virtual ~INLEquation() = default;
};

namespace detail
{
	struct NLEqnDeleter
	{
		void operator()(INLEquation* ptr) { if (ptr) { ptr->Destroy(); } };
	};
}

using NLEqPtr = std::unique_ptr<INLEquation, detail::NLEqnDeleter>;

template <typename T, typename...Args>
NLEqPtr make_NLEqn(Args&&...args)
{
	return NLEqPtr{ new T{std::forward<Args>(args)...} };
}

class FunctorNLEquation : public INLEquation
{
public:
	using NLFunctor = std::function<void(std::span<const double> input, std::span<double> output)>;

	explicit FunctorNLEquation(NLFunctor functor);
	[[nodiscard]] Status Apply(std::span<const double> input, std::span<double> output) override;
private:
	NLFunctor m_functor;
};

struct NLEquationWrapper;

struct NLEquationView
{
	std::span<const int32_t> input_ids;
	std::span<const int32_t> output_ids;
	NLEqPtr& equation;
	
	NLEquationWrapper ToFunctor() &&;
};

struct NLEquationWrapper
{
	std::vector<int32_t> input_ids;
	std::vector<int32_t> output_ids;
	NLEqPtr equation;

	operator NLEquationView()
	{
		return NLEquationView{ input_ids, output_ids, equation };
	}
};

inline NLEquationWrapper NLEquationView::ToFunctor() &&
{
	return {
			{input_ids.begin(), input_ids.end()},
			{output_ids.begin(), output_ids.end()},
			std::move(equation)
	};
}

class BufferEquation
{
public:
	int32_t input_id;
	int32_t output_id;
};

}