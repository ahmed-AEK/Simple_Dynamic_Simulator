#pragma once

#include "optimizer/Equation.hpp"

#include <memory>
#include <vector>
#include <memory_resource>
#include <functional>
#include <span>

namespace opt
{

	class IFlowEquation
	{
	public:
		[[nodiscard]] virtual Status Apply(std::span<const double> input, std::span<double> output) = 0;
		virtual void Destroy() { delete this; }
		virtual const char* GetLastError() { return "unknown error"; }
		virtual void ClearError() {}
	protected:
		virtual ~IFlowEquation() = default;
	};

	namespace detail
	{
		struct FlowEqnDeleter
		{
			void operator()(IFlowEquation* ptr) { if (ptr) { ptr->Destroy(); } };
		};
	}

	using FlowEqPtr = std::unique_ptr<IFlowEquation, detail::FlowEqnDeleter>;

	template <typename T, typename...Args>
	FlowEqPtr make_FlowEqn(Args&&...args)
	{
		return FlowEqPtr{ new T{std::forward<Args>(args)...} };
	}

	class FunctorFlowEquation : public IFlowEquation
	{
	public:
		using FlowFunctor = std::function<void(std::span<const double> input, std::span<double> output)>;

		explicit FunctorFlowEquation(FlowFunctor functor);
		[[nodiscard]] Status Apply(std::span<const double> input, std::span<double> output) override;
	private:
		FlowFunctor m_functor;
	};

	struct FlowEquationWrapper;

	struct FlowEquationView
	{
		std::span<const int32_t> input_ids;
		std::span<const int32_t> inout_ids;
		FlowEqPtr& equation;

		FlowEquationWrapper ToFunctor()&&;
	};

	struct FlowEquationWrapper
	{
		std::vector<int32_t> input_ids;
		std::vector<int32_t> inout_ids;
		FlowEqPtr equation;

		operator FlowEquationView()
		{
			return FlowEquationView{ input_ids, inout_ids, equation };
		}
	};

	inline FlowEquationWrapper FlowEquationView::ToFunctor()&&
	{
		return {
				{input_ids.begin(), input_ids.end()},
				{inout_ids.begin(), inout_ids.end()},
				std::move(equation)
		};
	}

}