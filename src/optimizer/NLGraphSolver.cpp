#include "NLGraphSolver.hpp"
#include "toolgui/NodeMacros.h"
#include <cassert>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <boost/range/adaptors.hpp>
#include <queue>
#include "optimizer/NLGraphSolver_private.hpp"

double opt::NLGraphSolver_impl::SolveInternal(std::span<const double> x, std::span<double> grad)
{
    UNUSED_PARAM(grad);
    LoadDatatoMap(x, m_current_state);
    double main_penalty = CalcPenalty(m_current_state);
    if (grad.size())
    {
        for (size_t i = 0; i < x.size(); i++)
        {
            const double old_value = m_current_state.get(m_estimated_output_ids[i]);
            const double delta = std::abs(old_value) * 1e-5 + 1e-5;
            m_current_state.modify(m_estimated_output_ids[i], old_value + delta );
            const double new_penalty = CalcPenalty(m_current_state);
            grad[i] = (new_penalty - main_penalty) / delta;
            m_current_state.modify(m_estimated_output_ids[i], old_value);
        }
    }
    return main_penalty;
}

double opt::NLGraphSolver_impl::CostFunction(unsigned n, const double* x, double* grad, void* data)
{
    UNUSED_PARAM(grad);
    opt::NLGraphSolver_impl* this_ptr = static_cast<opt::NLGraphSolver_impl*>(data);
    std::span<double> grad_span;
    if (grad)
    {
        grad_span = { grad, grad + n };
    }
    return this_ptr->SolveInternal({ x, x + n }, grad_span);
}

void opt::NLGraphSolver_impl::LoadDatatoMap(std::span<const double> x, FlatMap& state)
{
    assert(x.size() == m_estimated_output_ids.size());
    for (size_t i = 0; i < x.size(); i++)
    {
        state.modify(m_estimated_output_ids[i], x[i]);
    }
}

std::vector<double> opt::NLGraphSolver_impl::LoadMaptoVec(FlatMap& state)
{
    auto state_data = state.data();
    std::vector<double> output;
    output.reserve(state_data.size());
    for (size_t i = 0; i < m_estimated_output_ids.size(); i++)
    {
        output.push_back(state.get(m_estimated_output_ids[i]));
    }
    return output;
}

double opt::NLGraphSolver_impl::CalcPenalty(FlatMap& state)
{
    EvalSpecificFunctors(state, m_inner_solve_eqns);
    double penalty = 0.0;
    for (auto&& eq_idx : m_estimated_eqns)
    {
        switch (eq_idx.type)
        {
        case EquationType::NLEquation:
        {
            auto& eq = m_equations[eq_idx.index];
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
            for (size_t i = 0; i < output_buffer.size(); i++)
            {
                const double new_value = output_buffer[i];
                const auto old_value = state.get(output_ids[i]);
                penalty += (new_value - old_value) * (new_value - old_value);
            }
            break;
        }
        case EquationType::statefulNLEquation:
        {
            auto& eq = m_stateful_equations[eq_idx.index];
            const auto& n = eq_idx.index;
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
            break;
        }
        }
    }
    return std::sqrt(penalty);
}

void opt::NLGraphSolver_impl::UpdateStateInternal(FlatMap& state)
{
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
    }
}

opt::NLGraphSolver_impl::NLGraphSolver_impl(std::vector<NLEquation> equations)
    :m_equations(std::move(equations))
{
}

opt::NLGraphSolver_impl::NLGraphSolver_impl()
{
}

void opt::NLGraphSolver_impl::Initialize()
{
    std::vector<int64_t> remaining_output_ids;

    for (const auto& item : m_equations)
    {
		auto&& range = item.get_output_ids();
        std::transform(range.begin(), range.end(), std::back_inserter(remaining_output_ids),
            [](const auto& item) { return item; });
    }
    for (const auto& item : m_stateful_equations)
    {
		auto&& range = item.get_output_ids();
        std::transform(range.begin(), range.end(), std::back_inserter(remaining_output_ids),
            [](const auto& item) { return item; });
    }
    m_equations_states.resize(m_stateful_equations.size());

    FillInitialSolveEqns(remaining_output_ids);
    for (const auto& id: m_initial_solve_output_ids)
    {
        auto it = std::find(remaining_output_ids.begin(), remaining_output_ids.end(), id);
        assert(it != remaining_output_ids.end()); // id must be in this vector !
        if (it != remaining_output_ids.end())
        {
            remaining_output_ids.erase(it);
        }
    }
    if (remaining_output_ids.size())
    {
        FillInnerSolveEqns(remaining_output_ids);
    }
    m_optimizer = nlopt::opt(nlopt::LD_SLSQP, static_cast<unsigned int>(m_estimated_output_ids.size()));
    m_optimizer.set_min_objective(opt::NLGraphSolver_impl::CostFunction, this);
    m_optimizer.set_xtol_rel(1e-6);
    m_optimizer.set_xtol_abs(1e-8);
    m_optimizer.set_ftol_rel(1e-6);
    m_optimizer.set_ftol_abs(1e-8);
}

void opt::NLGraphSolver_impl::FillInitialSolveEqns(std::vector<int64_t>& remaining_output_ids)
{
    // khan's algorithm, we see which blocks have no unevaluated inputs, add them to inital functors, then see whether the connected blocks can be evaluated yet.
    struct BlockInputsCount
    {
        EquationIndex index;
        size_t count;
    };

    std::vector<BlockInputsCount> all_blocks;
    std::unordered_map<int64_t, std::vector<size_t>> edges; // map output_id -> all_blocks index
    std::queue<EquationIndex> blocks_to_process;

    for (size_t i = 0; i < m_equations.size(); i++)
    {
        all_blocks.push_back({ i, EquationType::NLEquation, 0 });
        auto&& inputs = m_equations[i].get_input_ids();
        for (auto&& input_id : inputs)
        {
            auto it = std::find(remaining_output_ids.begin(), remaining_output_ids.end(), input_id);
            if (it != remaining_output_ids.end())
            {
                all_blocks.back().count++;
            }
            edges[input_id].push_back(all_blocks.size() - 1);
        }
        if (all_blocks.back().count == 0)
        {
            blocks_to_process.push(all_blocks.back().index);
        }
    }
    for (size_t i = 0; i < m_stateful_equations.size(); i++)
    {
        all_blocks.push_back({ i, EquationType::statefulNLEquation, 0 });
        auto&& inputs = m_stateful_equations[i].get_input_ids();
        for (auto&& input_id : inputs)
        {
            auto it = std::find(remaining_output_ids.begin(), remaining_output_ids.end(), input_id);
            if (it != remaining_output_ids.end())
            {
                all_blocks.back().count++;
            }
            edges[input_id].push_back(all_blocks.size() - 1);
        }
        if (all_blocks.back().count == 0)
        {
            blocks_to_process.push(all_blocks.back().index);
        }
    }

    while (blocks_to_process.size())
    {
        auto index = blocks_to_process.front();
        blocks_to_process.pop();

        auto output_ids = [&]()->std::span<const int64_t>
            {
                switch (index.type)
                {
                case EquationType::NLEquation:
                {
                    return m_equations[index.index].get_output_ids();
                }
                case EquationType::statefulNLEquation:
                {
                    return m_stateful_equations[index.index].get_output_ids();
                }
                }
                return {};
            }();
        for (const auto& id : output_ids)
        {
            m_initial_solve_output_ids.push_back(id);
            auto it = edges.find(id);
            if (it == edges.end())
            {
                continue;
            }
            auto&& blocks = it->second;
            for (auto&& block : blocks)
            {
                all_blocks[block].count--;
                if (all_blocks[block].count == 0)
                {
                    blocks_to_process.push(all_blocks[block].index);
                }
            }
        }

        m_initial_solve_eqns.push_back(index);
    }
}

void opt::NLGraphSolver_impl::EvalSpecificFunctors(FlatMap& state, const std::vector<EquationIndex>& indicies)
{
    for (const auto& eq_index : indicies)
    {
        switch (eq_index.type)
        {
        case EquationType::NLEquation:
        {
            auto& eq = m_equations[eq_index.index];
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
            for (size_t i = 0; i < output_buffer.size(); i++)
            {
                const double new_value = output_buffer[i];
                state.modify(output_ids[i], new_value);
            }
            break;
        }
        case EquationType::statefulNLEquation:
        {
            auto& eq = m_stateful_equations[eq_index.index];
            const auto& n = eq_index.index;
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
                state.modify(output_ids[i], new_value);
            }
            break;
        }
        }
    }
}

void opt::NLGraphSolver_impl::FillInnerSolveEqns(std::vector<int64_t>& remaining_output_ids)
{
    struct BlockInputsCount
    {
        EquationIndex index;
        size_t count;
    };

    std::vector<BlockInputsCount> all_blocks;
    std::unordered_map<int64_t, std::vector<size_t>> edges; // map output_id -> all_blocks index

    for (size_t i = 0; i < m_equations.size(); i++)
    {
        all_blocks.push_back({ i, EquationType::NLEquation, 0 });
        auto&& inputs = m_equations[i].get_input_ids();
        for (auto&& input_id : inputs)
        {
            auto it = std::find(remaining_output_ids.begin(), remaining_output_ids.end(), input_id);
            if (it != remaining_output_ids.end())
            {
                all_blocks.back().count++;
            }
            edges[input_id].push_back(all_blocks.size() - 1);
        }
    }
    for (size_t i = 0; i < m_stateful_equations.size(); i++)
    {
        all_blocks.push_back({ i, EquationType::statefulNLEquation, 0 });
        auto&& inputs = m_stateful_equations[i].get_input_ids();
        for (auto&& input_id : inputs)
        {
            auto it = std::find(remaining_output_ids.begin(), remaining_output_ids.end(), input_id);
            if (it != remaining_output_ids.end())
            {
                all_blocks.back().count++;
            }
            edges[input_id].push_back(all_blocks.size() - 1);
        }
    }

    std::queue<EquationIndex> blocks_to_process;
    while (remaining_output_ids.size())
    {
        if (blocks_to_process.size())
        {
            auto index = blocks_to_process.front();
            blocks_to_process.pop();

            auto output_ids = [&]()->std::span<const int64_t>
                {
                    switch (index.type)
                    {
                    case EquationType::NLEquation:
                    {
                        return m_equations[index.index].get_output_ids();
                    }
                    case EquationType::statefulNLEquation:
                    {
                        return m_stateful_equations[index.index].get_output_ids();
                    }
                    }
                    return {};
                }();
                for (const auto& id : output_ids)
                {
                    m_inner_solve_output_ids.push_back(id);
                    {
                        auto it = std::find(remaining_output_ids.begin(), remaining_output_ids.end(), id);
                        assert(it != remaining_output_ids.end());
                        if (it != remaining_output_ids.end())
                        {
                            remaining_output_ids.erase(it);
                        }
                    }
                    auto it = edges.find(id);
                    if (it == edges.end())
                    {
                        continue;
                    }
                    auto&& blocks = it->second;
                    for (auto&& block : blocks)
                    {
                        all_blocks[block].count--;
                        if (all_blocks[block].count == 0)
                        {
                            blocks_to_process.push(all_blocks[block].index);
                        }
                    }
                }

                m_inner_solve_eqns.push_back(index);
        }
        else
        {
            // look for the block with highest input count, and least output count
            std::optional<BlockInputsCount> max_block;
            for (const auto& block : all_blocks)
            {
                if (!max_block.has_value() && block.count > 0)
                {
                    max_block = block;
                    continue;
                }
                if (block.count >= max_block->count)
                {
                    auto get_block_outputs_count = [&](const BlockInputsCount& block_idx)->size_t
                        {
                            switch (block_idx.index.type)
                            {
                            case EquationType::NLEquation:
                            {
                                return m_equations[block_idx.index.index].get_output_ids().size();
                            }
                            case EquationType::statefulNLEquation:
                            {
                                return m_stateful_equations[block_idx.index.index].get_output_ids().size();
                            }
                            }
                            return 0;
                        };
                    size_t first_output_ids_count = get_block_outputs_count(*max_block);
                    size_t second_output_ids_count = get_block_outputs_count(block);
                    if (second_output_ids_count < first_output_ids_count)
                    {
                        max_block = block;
                    }
                    
                }
            }
            assert(max_block);

            // add it as estimated block
            m_estimated_eqns.push_back(max_block->index);
            auto output_ids = [&]()->std::span<const int64_t>
                {
                    switch (max_block->index.type)
                    {
                    case EquationType::NLEquation:
                    {
                        return m_equations[max_block->index.index].get_output_ids();
                    }
                    case EquationType::statefulNLEquation:
                    {
                        return m_stateful_equations[max_block->index.index].get_output_ids();
                    }
                    }
                    return {};
                }();
            for (const auto& id : output_ids)
            {
                m_estimated_output_ids.push_back(id);
                {
                    auto it = std::find(remaining_output_ids.begin(), remaining_output_ids.end(), id);
                    assert(it != remaining_output_ids.end());
                    if (it != remaining_output_ids.end())
                    {
                        remaining_output_ids.erase(it);
                    }
                }
                auto it = edges.find(id);
                if (it == edges.end())
                {
                    continue;
                }
                auto&& blocks = it->second;
                for (auto&& block : blocks)
                {
                    all_blocks[block].count--;
                    if (all_blocks[block].count == 0)
                    {
                        blocks_to_process.push(all_blocks[block].index);
                    }
                }
            }
        }
    }
}

void opt::NLGraphSolver_impl::AddEquation(opt::NLEquation eq)
{
    m_equations.emplace_back(std::move(eq));
}

void opt::NLGraphSolver_impl::AddStatefulEquation(NLStatefulEquation eq)
{
    m_stateful_equations.push_back(eq);
}

static void OffloadSpecificIndicies(const opt::FlatMap& src, opt::FlatMap& dst, const std::vector<int64_t>& indicies)
{
    for (const auto& idx : indicies)
    {
        dst.modify(idx, src.get(idx));
    }
}

void opt::NLGraphSolver_impl::Solve(FlatMap& state, const double& time)
{
    if ( 0 == m_estimated_output_ids.size() && 0 == m_initial_solve_output_ids.size())
    {
        return;
    }

    m_current_time = time;
    opt::FlatMap::sync(state, m_current_state);
    EvalSpecificFunctors(m_current_state, m_initial_solve_eqns);
    if (m_estimated_output_ids.size())
    {
        std::vector<double> x = LoadMaptoVec(m_current_state);
        double min_value;
        [[maybe_unused]] nlopt::result result = m_optimizer.optimize(x, min_value);
        EvalSpecificFunctors(m_current_state, m_inner_solve_eqns);

        OffloadSpecificIndicies(m_current_state, state, m_initial_solve_output_ids);
        OffloadSpecificIndicies(m_current_state, state, m_inner_solve_output_ids);
        for (size_t idx = 0; idx < m_estimated_output_ids.size(); idx++)
        {
            state.modify(m_estimated_output_ids[idx], x[idx]);
        }
    }
    else
    {
        OffloadSpecificIndicies(m_current_state, state, m_initial_solve_output_ids);
    }
    //LoadDatatoMap(x, state);
}

void opt::NLGraphSolver_impl::UpdateState(FlatMap& state, const double& time)
{
    if (0 == m_estimated_output_ids.size() && 0 == m_initial_solve_output_ids.size())
    {
        return;
    }

    m_current_time = time;
    opt::FlatMap::sync(state, m_current_state);

    UpdateStateInternal(state);
}

opt::NLGraphSolver::NLGraphSolver()
    :m_impl{std::make_unique<NLGraphSolver_impl>()}
{
}

opt::NLGraphSolver::~NLGraphSolver() = default;

opt::NLGraphSolver::NLGraphSolver(NLGraphSolver&&) noexcept = default;

opt::NLGraphSolver& opt::NLGraphSolver::operator=(NLGraphSolver&&) noexcept = default;

void opt::NLGraphSolver::Initialize()
{
    m_impl->Initialize();
}

void opt::NLGraphSolver::Solve(FlatMap& state, const double& time)
{
    m_impl->Solve(state, time);
}

void opt::NLGraphSolver::UpdateState(FlatMap& state, const double& time)
{
    m_impl->UpdateState(state, time);
}

void opt::NLGraphSolver::AddEquation(NLEquation eq)
{
    m_impl->AddEquation(std::move(eq));
}

void opt::NLGraphSolver::AddStatefulEquation(NLStatefulEquation eq)
{
    m_impl->AddStatefulEquation(std::move(eq));
}
