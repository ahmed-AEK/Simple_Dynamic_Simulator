#include "NLSolver.hpp"
#include "toolgui/NodeMacros.h"
#include <cassert>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <boost/range/adaptors.hpp>

double opt::NLSolver::SolveInternal(std::span<const double> x, std::span<double> grad)
{
    UNUSED_PARAM(grad);
    LoadDatatoMap(x, m_current_state);
    double main_penalty = CalcPenalty(m_current_state);
    if (grad.size())
    {
        for (size_t i = 0; i < x.size(); i++)
        {
            const double old_value = m_current_state.get(m_output_ids[i]);
            const double delta = std::abs(old_value) * 1e-5 + 1e-5;
            m_current_state.modify(m_output_ids[i], old_value + delta );
            const double new_penalty = CalcPenalty(m_current_state);
            grad[i] = (new_penalty - main_penalty) / delta;
            m_current_state.modify(m_output_ids[i], old_value);
        }
    }
    return main_penalty;
}

double opt::NLSolver::CostFunction(unsigned n, const double* x, double* grad, void* data)
{
    UNUSED_PARAM(grad);
    opt::NLSolver* this_ptr = static_cast<opt::NLSolver*>(data);
    std::span<double> grad_span;
    if (grad)
    {
        grad_span = { grad, grad + n };
    }
    return this_ptr->SolveInternal({ x, x + n }, grad_span);
}

void opt::NLSolver::LoadDatatoMap(std::span<const double> x, FlatMap& state)
{
    assert(x.size() == m_output_ids.size());
    for (size_t i = 0; i < x.size(); i++)
    {
        state.modify(m_output_ids[i], x[i]);
    }
}

std::vector<double> opt::NLSolver::LoadMaptoVec(FlatMap& state)
{
    auto state_data = state.data();
    std::vector<double> output;
    output.reserve(state_data.size());
    for (size_t i = 0; i < m_output_ids.size(); i++)
    {
        output.push_back(state.get(m_output_ids[i]));
    }
    return output;
}

double opt::NLSolver::CalcPenalty(FlatMap& state)
{
    double penalty = 0.0;
    for (auto& eq : m_equations)
    {
        auto input_buffer = eq.get_input_buffer();
        auto input_ids = eq.get_input_ids();
        assert(input_buffer.size() == input_ids.size());
        for (size_t i = 0; i < input_buffer.size(); i++)
        {
            const auto val = state.get(input_ids[i]);
            input_buffer[i] = val;
        }
        eq.Apply();
        auto output_buffer = eq.get_output_buffer();
        auto output_ids = eq.get_output_ids();
        assert(output_buffer.size() == output_ids.size());
        for (size_t  i = 0; i < output_buffer.size(); i++)
        {
            const double new_value = output_buffer[i];
            const auto old_value = state.get(output_ids[i]);
            penalty += (new_value - old_value) * (new_value - old_value);
        }
    }

    for (auto element : m_stateful_equations | boost::adaptors::indexed())
    {
        auto& eq = element.value();
        const auto& n = element.index();
        auto input_buffer = eq.get_input_buffer();
        auto input_ids = eq.get_input_ids();
        assert(input_buffer.size() == input_ids.size());
        for (size_t i = 0; i < input_buffer.size(); i++)
        {
            const auto val = state.get(input_ids[i]);
            input_buffer[i] = val;
        }
        [[maybe_unused]] auto next_state = eq.Apply(m_current_time, m_equations_states[n]);
        auto output_buffer = eq.get_output_buffer();
        auto output_ids = eq.get_output_ids();
        assert(output_buffer.size() == output_ids.size());
        for (size_t i = 0; i < output_buffer.size(); i++)
        {
            const double new_value = output_buffer[i];
            const auto old_value = state.get(output_ids[i]);
            penalty += (new_value - old_value) * (new_value - old_value);
        }
    }
    return std::sqrt(penalty);
}

void opt::NLSolver::UpdateState(std::span<const double> x, FlatMap& state)
{
    LoadDatatoMap(x, state);
    for (auto& eq : m_equations)
    {
        auto input_buffer = eq.get_input_buffer();
        auto input_ids = eq.get_input_ids();
        assert(input_buffer.size() == input_ids.size());
        for (size_t i = 0; i < input_buffer.size(); i++)
        {
            const auto val = state.get(input_ids[i]);
            input_buffer[i] = val;
        }
        eq.Apply();
        auto output_buffer = eq.get_output_buffer();
        auto output_ids = eq.get_output_ids();
        assert(output_buffer.size() == output_ids.size());
        for (size_t i = 0;i < output_buffer.size(); i++)
        {
            const double result = output_buffer[i];
            state.modify(output_ids[i], result);
        }
    }
    for (auto element : m_stateful_equations | boost::adaptors::indexed())
    {
        auto& eq = element.value();
        const auto& n = element.index();
        auto input_buffer = eq.get_input_buffer();
        auto input_ids = eq.get_input_ids();
        assert(input_buffer.size() == input_ids.size());
        for (size_t i = 0; i < input_buffer.size(); i++)
        {
            const auto val = state.get(input_ids[i]);
            input_buffer[i] = val;
        }
        m_equations_states[n] = eq.Apply(m_current_time, m_equations_states[n]);
        auto output_buffer = eq.get_output_buffer();
        auto output_ids = eq.get_output_ids();
        assert(output_buffer.size() == output_ids.size());
        for (size_t i = 0; i < output_buffer.size(); i++)
        {
            const double result = output_buffer[i];
            state.modify(output_ids[i], result);
        }
    }
}

opt::NLSolver::NLSolver(std::vector<NLEquation> equations)
    :m_equations(std::move(equations))
{
}

opt::NLSolver::NLSolver()
{
}

void opt::NLSolver::Initialize()
{
    m_output_ids.clear();

    for (const auto& item : m_equations)
    {
		auto&& range = item.get_output_ids();
        std::transform(range.begin(), range.end(), std::back_inserter(m_output_ids), 
            [](const auto& item) { return item; });
    }
    for (const auto& item : m_stateful_equations)
    {
		auto&& range = item.get_output_ids();
        std::transform(range.begin(), range.end(), std::back_inserter(m_output_ids),
            [](const auto& item) { return item; });
    }
    m_equations_states.resize(m_stateful_equations.size());
    m_optimizer = nlopt::opt(nlopt::LD_SLSQP, static_cast<unsigned int>(m_output_ids.size()));
    m_optimizer.set_min_objective(opt::NLSolver::CostFunction, this);
    m_optimizer.set_xtol_rel(1e-6);
    m_optimizer.set_xtol_abs(1e-8);
    m_optimizer.set_ftol_rel(1e-6);
    m_optimizer.set_ftol_abs(1e-8);
}

void opt::NLSolver::AddEquation(opt::NLEquation eq)
{
    m_equations.emplace_back(std::move(eq));
}

void opt::NLSolver::AddStatefulEquation(NLStatefulEquation eq)
{
    m_stateful_equations.push_back(eq);
}

void opt::NLSolver::Solve(FlatMap& state, const double& time)
{
    if ( 0 == m_output_ids.size())
    {
        return;
    }

    m_current_time = time;
    opt::FlatMap::sync(state, m_current_state);
    std::vector<double> x = LoadMaptoVec(state);
    double min_value;
    [[maybe_unused]] nlopt::result result = m_optimizer.optimize(x, min_value);

    UpdateState(x, state);
}
