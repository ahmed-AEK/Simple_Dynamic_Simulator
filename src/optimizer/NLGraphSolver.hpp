#pragma once

#include "optimizer/NLEquation.hpp"
#include "optimizer/NLStatefulEquation.hpp"
#include "optimizer/FlowEquation.hpp"
#include "optimizer/PotentialEquation.hpp"

#include "optimizer/flatmap.hpp"
#include <memory>
#include <tl/expected.hpp>
#include <variant>

namespace opt
{

class NLGraphSolver_impl;

using NLSolveResult = tl::expected<std::monostate, std::string>;

class NLGraphSolver
{
public:
	NLGraphSolver();
	~NLGraphSolver();
	NLGraphSolver(NLGraphSolver&&) noexcept;
	NLGraphSolver& operator=(NLGraphSolver&&) noexcept;

	[[nodiscard]] tl::expected<std::monostate, std::string> Initialize(std::span<int32_t> fixed_ids = {});
	[[nodiscard]] NLSolveResult Solve(FlatMap& state, const double& time);
	[[nodiscard]] NLSolveResult UpdateState(FlatMap& state, const double& time);
	void AddEquation(NLEquationWrapper eq);
	void AddStatefulEquation(NLStatefulEquationWrapper eq);
	void AddBufferEquation(BufferEquation eq);
	void AddFlowEquation(FlowEquationWrapper eq);
	void AddPotentialEquation(PotentialEquationWrapper eq);
	std::vector<NLStatefulEquationWrapper>& GetStatefulEquations();
private:
	std::unique_ptr<NLGraphSolver_impl> m_impl;
};

}