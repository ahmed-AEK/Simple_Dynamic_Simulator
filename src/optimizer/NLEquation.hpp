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
	virtual void Apply(std::span<const double> input, std::span<double> output) = 0;
	virtual void Destroy() { delete this; }
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
	using NLFunctor = std::function<void(std::span<const double>, std::span<double>)>;

	FunctorNLEquation(NLFunctor functor);
	void Apply(std::span<const double> input, std::span<double> output) override;
private:
	NLFunctor m_functor;
};

struct NLEquationWrapper
{
	std::vector<int32_t> input_ids;
	std::vector<int32_t> output_ids;
	NLEqPtr equation;
};

class BufferEquation
{
public:
	int32_t input_id;
	int32_t output_id;
};

}