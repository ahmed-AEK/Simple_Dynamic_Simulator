#pragma once

#include "optimizer/NLSolver.hpp"
#include "optimizer/DiffSolver.hpp"
#include "optimizer/Observer.hpp"

namespace opt
{

class NLDiffSolver
{
public:
	NLDiffSolver();
	void AddDiffEquation(DiffEquation eq);
	void AddNLEquation(NLEquation eq);
	void AddNLStatefulEquation(NLStatefulEquation eq);
	void AddObserver(Observer obs);
	void Initialize(double start_time, double end_time);
	[[nodiscard]] constexpr double GetStartTime() const { return m_diffSolver.GetStartTime(); }
	[[nodiscard]] constexpr double GetEndTime() const { return m_diffSolver.GetEndTime(); }
	[[nodiscard]] constexpr double GetCurrentTime() const { return m_diffSolver.GetCurrentTime(); }
	void CalculateInitialConditions(FlatMap& state);
	void NotifyObservers(const FlatMap& state, const double t);
	StepResult Step(FlatMap& state);
private:
	DiffSolver m_diffSolver;
	NLSolver m_NLSolver;
	std::vector<Observer> m_observers;
};
}