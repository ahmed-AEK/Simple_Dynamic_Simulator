#pragma once

#include "optimizer/NLEquation.hpp"
#include "optimizer/NLStatefulEquation.hpp"
#include "optimizer/flatmap.hpp"
#include <nlopt.hpp>

namespace opt
{

class NLGraphSolver
{
public:
	explicit NLGraphSolver(std::vector<NLEquation> equations);
	NLGraphSolver();
	void Initialize();
	void Solve(FlatMap& state, const double& time);
	void UpdateState(FlatMap& state, const double& time);
	void AddEquation(NLEquation eq);
	void AddStatefulEquation(NLStatefulEquation eq);
protected:
	[[nodiscard]] double SolveInternal(std::span<const double> x, std::span<double> grad);
	static double CostFunction(unsigned n, const double* x, double* grad, void* data);
	void LoadDatatoMap(std::span<const double> x, FlatMap& state);
	[[nodiscard]] std::vector<double> LoadMaptoVec(FlatMap& state);
	[[nodiscard]] double CalcPenalty(FlatMap& state);
	void UpdateStateInternal(FlatMap& state);
private:

	

	std::vector<NLEquation> m_equations;
	std::vector<NLStatefulEquation> m_stateful_equations;
	std::vector<FatAny> m_equations_states;
	opt::FlatMap m_current_state;
	double m_current_time = 0;
	nlopt::opt m_optimizer;

	enum class EquationType
	{
		NLEquation,
		statefulNLEquation,
	};
	struct EquationIndex
	{
		size_t index;
		EquationType type;
	};

	std::vector<EquationIndex> m_initial_solve_eqns;
	std::vector<int64_t> m_initial_solve_output_ids;
	void FillInitialSolveEqns(std::vector<int64_t>& remaining_output_ids);
	void EvalSpecificFunctors(FlatMap& state, const std::vector<EquationIndex>& indicies);

	std::vector<EquationIndex> m_estimated_eqns;
	std::vector<int64_t> m_estimated_output_ids;
	std::vector<EquationIndex> m_inner_solve_eqns;
	std::vector<int64_t> m_inner_solve_output_ids;
	void FillInnerSolveEqns(std::vector<int64_t>& remaining_output_ids);
};

}