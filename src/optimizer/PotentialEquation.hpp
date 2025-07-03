#pragma once

#include "optimizer/Equation.hpp"

#include <memory>
#include <vector>
#include <memory_resource>
#include <functional>
#include <span>

namespace opt
{

	class IPotentialEquation
	{
	public:
		[[nodiscard]] virtual Status Apply(std::span<const double> input, double flow, double& potential) = 0;
		virtual void Destroy() { delete this; }
		virtual const char* GetLastError() { return "unknown error"; }
		virtual void ClearError() {}
	protected:
		virtual ~IPotentialEquation() = default;
	};

	namespace detail
	{
		struct PotentialEqnDeleter
		{
			void operator()(IPotentialEquation* ptr) { if (ptr) { ptr->Destroy(); } };
		};
	}

	using PotentialEqPtr = std::unique_ptr<IPotentialEquation, detail::PotentialEqnDeleter>;

	template <typename T, typename...Args>
	PotentialEqPtr make_PotentialEqn(Args&&...args)
	{
		return PotentialEqPtr{ new T{std::forward<Args>(args)...} };
	}

	class FunctorPotentialEquation : public IPotentialEquation
	{
	public:
		using PotentialFunctor = std::function<void(std::span<const double> input, double flow, double& potential)>;

		explicit FunctorPotentialEquation(PotentialFunctor functor);
		[[nodiscard]] Status Apply(std::span<const double> input, double flow, double& potential) override;
	private:
		PotentialFunctor m_functor;
	};

	struct PotentialEquationWrapper;

	struct PotentialEquationView
	{
		std::span<const int32_t> input_ids;
		std::span<const int32_t> inout_ids;
		const int32_t& flow_value_id;
		PotentialEqPtr& equation;

		PotentialEquationWrapper ToFunctor()&&;
	};

	struct PotentialEquationWrapper
	{
		std::vector<int32_t> input_ids;
		std::array<int32_t, 2> inout_ids;
		int32_t flow_value_id;
		PotentialEqPtr equation;

		operator PotentialEquationView()
		{
			return PotentialEquationView{ input_ids, inout_ids, flow_value_id, equation };
		}
	};

	inline PotentialEquationWrapper PotentialEquationView::ToFunctor()&&
	{
		return {
				{input_ids.begin(), input_ids.end()},
				{inout_ids[0], inout_ids[1]},
				flow_value_id,
				std::move(equation)
		};
	}

}