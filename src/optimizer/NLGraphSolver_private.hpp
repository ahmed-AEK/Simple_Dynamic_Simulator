#pragma once

#include "optimizer/NLEquation.hpp"
#include "optimizer/NLStatefulEquation.hpp"
#include "optimizer/flatmap.hpp"
#include <nlopt.hpp>
#include <tl/expected.hpp>

namespace opt
{

class NLGraphSolver_impl
{
public:
	explicit NLGraphSolver_impl(std::vector<NLEquationWrapper> equations);
	NLGraphSolver_impl();
	void Initialize();
	[[nodiscard]] NLSolveResult Solve(FlatMap& state, const double& time);
	[[nodiscard]] NLSolveResult UpdateState(FlatMap& state, const double& time);
	void AddEquation(NLEquationWrapper eq);
	void AddStatefulEquation(NLStatefulEquationWrapper eq);
	void AddBufferEq(BufferEquation eq);
	std::vector<NLStatefulEquationWrapper>& GetStatefulEquations();
protected:
	[[nodiscard]] tl::expected<double, std::string> SolveInternal(std::span<const double> x, std::span<double> grad);
	static double CostFunction(unsigned n, const double* x, double* grad, void* data);
	void LoadDatatoMap(std::span<const double> x, FlatMap& state);
	[[nodiscard]] std::vector<double> LoadMaptoVec(FlatMap& state);
	[[nodiscard]] tl::expected<double, std::string> CalcPenalty(FlatMap& state);
	[[nodiscard]] NLSolveResult UpdateStateInternal(FlatMap& state);

private:

	std::vector<NLEquationWrapper> m_equations;
	std::vector<NLStatefulEquationWrapper> m_stateful_equations;
	std::vector<BufferEquation> m_buffer_equations;
	double m_last_state_time = 0;
	opt::FlatMap m_current_state;
	double m_current_time = 0;
	nlopt::opt m_optimizer;
	std::optional<std::string> m_last_error;

	enum class EquationType
	{
		NLEquation,
		statefulNLEquation,
		BufferEquation,
	};
	struct EquationIndex
	{
		size_t index;
		EquationType type;
	};

	std::vector<EquationIndex> m_initial_solve_eqns;
	std::vector<int32_t> m_initial_solve_output_ids;
	void FillInitialSolveEqns(std::vector<int32_t>& remaining_output_ids);
	[[nodiscard]] NLSolveResult EvalSpecificFunctors(FlatMap& state, const std::vector<EquationIndex>& indicies);

	std::vector<EquationIndex> m_estimated_eqns;
	std::vector<int32_t> m_estimated_output_ids;
	std::vector<EquationIndex> m_inner_solve_eqns;
	std::vector<int32_t> m_inner_solve_output_ids;
	void FillInnerSolveEqns(std::vector<int32_t>& remaining_output_ids);
};

}