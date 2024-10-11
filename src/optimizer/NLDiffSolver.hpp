#pragma once

#include "optimizer/NLGraphSolver.hpp"
#include "optimizer/DiffSolver.hpp"
#include <any>

namespace opt
{
class Observer;
class SourceEq;

struct ObserverSlot;

struct ObserverData
{
	size_t id;
	std::any data;
};

class NLDiffSolver
{
public:
	NLDiffSolver();
	NLDiffSolver(const NLDiffSolver&) = delete;
	NLDiffSolver& operator=(const NLDiffSolver&) = delete;
	~NLDiffSolver();
	void SetMaxStep(double step_size);
	void AddDiffEquation(DiffEquation eq);
	void AddNLEquation(NLEquation eq);
	void AddNLStatefulEquation(NLStatefulEquation eq);
	size_t AddObserver(Observer obs);
	void AddSource(SourceEq source);
	void Initialize(double start_time, double end_time);
	[[nodiscard]] double GetStartTime() const { return m_diffSolver.GetStartTime(); }
	[[nodiscard]] double GetEndTime() const { return m_diffSolver.GetEndTime(); }
	[[nodiscard]] double GetCurrentTime() const { return m_diffSolver.GetCurrentTime(); }
	void CalculateInitialConditions(FlatMap& state);
	void NotifyObservers(const FlatMap& state, const double t);
	std::vector<ObserverData> GetObserversData();
	StepResult Step(FlatMap& state);
private:
	void ApplySources(FlatMap& state, const double t);
	void UpdateSources(const double t);
	double m_last_oberver_time = 0;
	std::vector<SourceEq> m_sources;
	DiffSolver m_diffSolver;
	NLGraphSolver m_NLSolver;
	std::vector<ObserverSlot> m_observers;
	std::vector<double> m_source_event_times;
};
}