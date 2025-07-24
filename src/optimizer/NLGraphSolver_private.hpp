#pragma once

#include "optimizer/NLEquation.hpp"
#include "optimizer/NLStatefulEquation.hpp"
#include "optimizer/FlowEquation.hpp"
#include "optimizer/PotentialEquation.hpp"

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
	[[nodiscard]] tl::expected<std::monostate, std::string> Initialize(std::span<int32_t> fixed_ids);
	[[nodiscard]] NLSolveResult Solve(FlatMap& state, const double& time);
	[[nodiscard]] NLSolveResult UpdateState(FlatMap& state, const double& time);
	void AddEquation(NLEquationWrapper eq);
	void AddStatefulEquation(NLStatefulEquationWrapper eq);
	void AddBufferEq(BufferEquation eq);
	void AddFlowEquation(FlowEquationWrapper eq);
	void AddPotentialEquation(PotentialEquationWrapper eq);

	std::vector<NLStatefulEquationWrapper>& GetStatefulEquations();
protected:
	[[nodiscard]] tl::expected<double, std::string> SolveInternal(std::span<const double> x, std::span<double> grad);
	static double CostFunction(unsigned n, const double* x, double* grad, void* data);
	void LoadDatatoMap(std::span<const double> x, FlatMap& state);
	void LoadMaptoVec(FlatMap& state, std::vector<double>& output);
	[[nodiscard]] tl::expected<double, std::string> CalcPenalty(FlatMap& state);
	[[nodiscard]] NLSolveResult UpdateStateInternal(FlatMap& state);

private:
	
	// equations
	std::vector<NLEquationWrapper> m_equations;
	std::vector<NLStatefulEquationWrapper> m_stateful_equations;
	std::vector<BufferEquation> m_buffer_equations;
	std::vector<FlowEquationWrapper> m_flow_equations;
	std::vector<PotentialEquationWrapper> m_potential_equations;

	// per step state
	double m_last_state_time = 0;
	opt::FlatMap m_current_state;
	double m_current_time = 0;
	nlopt::opt m_optimizer;
	std::optional<std::string> m_last_error;
	std::vector<double> m_current_x;

	enum class EquationType
	{
		NLEquation,
		statefulNLEquation,
		BufferEquation,
		FlowEquation,
		PotentialEquation,
	};
	struct EquationIndex
	{
		size_t index;
		EquationType type;
	};

	struct FlowNode
	{
		int32_t node_id;
		bool fixed;
		double current_value;
	};

	void FillInitialSolveEqns(std::set<int32_t>& remaining_output_ids);
	[[nodiscard]] NLSolveResult EvalSpecificFunctors(FlatMap& state, const std::vector<EquationIndex>& indicies);
	[[nodiscard]] NLSolveResult EvalFlowEquations(FlatMap& state, std::span<FlowNode> nodes);
	[[nodiscard]] tl::expected<std::monostate, std::string> FillInnerSolveEqns(std::set<int32_t>& remaining_output_ids);

	// solver configuration
	std::vector<EquationIndex> m_initial_solve_eqns;
	std::vector<int32_t> m_initial_solve_output_ids;
	std::vector<FlowNode> m_flow_nodes;
	std::vector<EquationIndex> m_estimated_eqns;
	std::vector<int32_t> m_estimated_output_ids;
	std::vector<EquationIndex> m_inner_solve_eqns;
	std::vector<int32_t> m_inner_solve_output_ids;
	std::vector<int32_t> m_estimated_flow_nodes;
	std::vector<int32_t> m_potential_eqn_solved_nodes;
	std::vector<int32_t> m_potential_eqn_id_to_output_id;
	std::vector<int32_t> m_node_id_to_flow_index;
};

}