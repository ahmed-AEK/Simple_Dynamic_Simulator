#pragma once

#include "optimizer/NLGraphSolver.hpp"
#include "optimizer/DiffSolver.hpp"
#include <any>

namespace opt
{
struct ObserverWrapper;
struct SourceEqWrapper;

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
	void AddDiffEquation(DiffEquationWrapper eq);
	void AddNLEquation(NLEquationWrapper eq);
	void AddNLStatefulEquation(NLStatefulEquationWrapper eq);
	void AddBufferEquation(BufferEquation eq);
	size_t AddObserver(ObserverWrapper obs);
	void AddSource(SourceEqWrapper source);
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
	void TriggerSources(const double t);
	void NotifyZeroCrossings(const FlatMap& state, const double t);
	bool UpdateNewZeroCrossings(const FlatMap& state);
	void InterpolateStateAt(FlatMap& state, const double t);

	static constexpr int ZeroCrossingIterations = 10;
	double m_last_oberver_time = 0;
	std::vector<SourceEqWrapper> m_sources;
	DiffSolver m_diffSolver;
	NLGraphSolver m_NLSolver;
	std::vector<ObserverSlot> m_observers;
	std::vector<double> m_source_event_times;
	std::vector<int32_t> m_new_zero_crossed_blocks;
	FlatMap m_crossed_states;
	FlatMap m_new_crossed_states;
};
}