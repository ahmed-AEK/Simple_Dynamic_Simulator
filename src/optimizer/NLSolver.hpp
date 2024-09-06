#pragma once

#include "optimizer/NLEquation.hpp"
#include "optimizer/NLStatefulEquation.hpp"
#include "optimizer/flatmap.hpp"
#include <nlopt.hpp>

namespace opt
{

class NLSolver
{
public:
	explicit NLSolver(std::vector<NLEquation> equations);
	NLSolver();
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
	std::vector<int64_t> m_output_ids;
	nlopt::opt m_optimizer;
};
}