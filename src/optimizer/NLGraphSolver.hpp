#pragma once

#include "optimizer/NLEquation.hpp"
#include "optimizer/NLStatefulEquation.hpp"
#include "optimizer/flatmap.hpp"
#include <memory>

namespace opt
{

class NLGraphSolver_impl;

class NLGraphSolver
{
public:
	NLGraphSolver();
	~NLGraphSolver();
	NLGraphSolver(NLGraphSolver&&) noexcept;
	NLGraphSolver& operator=(NLGraphSolver&&) noexcept;

	void Initialize();
	void Solve(FlatMap& state, const double& time);
	void UpdateState(FlatMap& state, const double& time);
	void AddEquation(NLEquationWrapper eq);
	void AddStatefulEquation(NLStatefulEquationWrapper eq);
	void AddBufferEquation(BufferEquation eq);
	
	std::vector<NLStatefulEquationWrapper>& GetStatefulEquations();
private:
	std::unique_ptr<NLGraphSolver_impl> m_impl;
};

}