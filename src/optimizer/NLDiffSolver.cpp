#include "NLDiffSolver.hpp"
#include "DiffEquation.hpp"
#include "NLEquation.hpp"
#include "toolgui/NodeMacros.h"

opt::NLDiffSolver::NLDiffSolver()
{
	auto preprocessor = [this](auto& state, auto time)
		{
			m_NLSolver.Solve(state, time);
		};
	m_diffSolver.SetPreprocessor(std::move(preprocessor));
}

void opt::NLDiffSolver::AddDiffEquation(DiffEquation eq)
{
	m_diffSolver.AddEquation(std::move(eq));
}

void opt::NLDiffSolver::AddNLEquation(NLEquation eq)
{
	m_NLSolver.AddEquation(std::move(eq));
}

void opt::NLDiffSolver::AddObserver(Observer obs)
{
	m_observers.push_back(std::move(obs));
}

void opt::NLDiffSolver::Initialize(double start_time, double end_time)
{
	m_diffSolver.Initialize(start_time, end_time);
	m_NLSolver.Initialize();
}

void opt::NLDiffSolver::CalculateInitialConditions(FlatMap& state)
{
	m_NLSolver.Solve(state, GetCurrentTime());
	NotifyObservers(state, GetCurrentTime());
}

void opt::NLDiffSolver::NotifyObservers(const FlatMap& state, const double t)
{
	for (auto& observer : m_observers)
	{
		auto input_buffer = observer.get_input_buffer();
		auto input_ids = observer.get_input_ids();
		assert(input_buffer.size() == input_ids.size());
		for (size_t i = 0; i < input_buffer.size(); i++)
		{
			const auto val = state.get(input_ids[i]);
			input_buffer[i] = val;
		}
		observer.Apply(t);
	}
}

opt::StepResult opt::NLDiffSolver::Step(FlatMap& state)
{
	const auto result = m_diffSolver.Step(state);
	m_NLSolver.Solve(state, GetCurrentTime());
	NotifyObservers(state, GetCurrentTime());
	return result;
}
