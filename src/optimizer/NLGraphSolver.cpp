#include "NLGraphSolver.hpp"
#include "toolgui/NodeMacros.h"
#include <boost/range/adaptors.hpp>
#include "optimizer/NLGraphSolver_private.hpp"

tl::expected<double, std::string> opt::NLGraphSolver_impl::SolveInternal(std::span<const double> x, std::span<double> grad)
{
    UNUSED_PARAM(grad);
    LoadDatatoMap(x, m_current_state);
    auto res_temp = CalcPenalty(m_current_state);
    if (!res_temp)
    {
        return res_temp;
    }
    double main_penalty = *res_temp;
    if (grad.size())
    {
        for (size_t i = 0; i < x.size(); i++)
        {
            const double old_value = m_current_state.get(m_estimated_output_ids[i]);
            const double delta = std::abs(old_value) * 1e-3 + 1e-5;
            m_current_state.modify(m_estimated_output_ids[i], old_value + delta );
            auto res = CalcPenalty(m_current_state);
            if (!res)
            {
                return res;
            }
            const double new_penalty = *res;
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
    auto result = this_ptr->SolveInternal({ x, x + n }, grad_span);
    if (!result)
    {
        this_ptr->m_last_error = std::move(result.error());
        throw nlopt::forced_stop{};
    }
    return *result;
}

void opt::NLGraphSolver_impl::LoadDatatoMap(std::span<const double> x, FlatMap& state)
{
    assert(x.size() == m_estimated_output_ids.size());
    for (size_t i = 0; i < x.size(); i++)
    {
        state.modify(m_estimated_output_ids[i], x[i]);
    }
}

void opt::NLGraphSolver_impl::LoadMaptoVec(FlatMap& state, std::vector<double>& output)
{
    auto state_data = state.data();
    output.clear();
    output.reserve(state_data.size());
    for (size_t i = 0; i < m_estimated_output_ids.size(); i++)
    {
        output.push_back(state.get(m_estimated_output_ids[i]));
    }
}

tl::expected<double, std::string> opt::NLGraphSolver_impl::CalcPenalty(FlatMap& state)
{
    {
        auto res = EvalSpecificFunctors(state, m_inner_solve_eqns);
        if (!res)
        {
            return tl::unexpected<std::string>{ std::move(res.error()) };
        }
    }
    double penalty = 0.0;
    std::array<double, 20> input_temp_buffer;
    std::array<double, 20> output_temp_buffer;
    for (auto&& eq_idx : m_estimated_eqns)
    {
        switch (eq_idx.type)
        {
        case EquationType::NLEquation:
        {
            auto& eq = m_equations[eq_idx.index];
            auto& input_ids = eq.input_ids;
            auto input_buffer = std::span{ input_temp_buffer }.subspan(0,input_ids.size());
            for (size_t i = 0; i < input_buffer.size(); i++)
            {
                const auto val = state.get(input_ids[i]);
                input_buffer[i] = val;
            }
            auto& output_ids = eq.output_ids;
            auto output_buffer = std::span{ output_temp_buffer }.subspan(0, output_ids.size());

            auto res = eq.equation->Apply(input_buffer, output_buffer);
            if (res == Status::error)
            {
                return tl::unexpected<std::string>{ eq.equation->GetLastError() };
            }
            for (size_t i = 0; i < output_buffer.size(); i++)
            {
                const double new_value = output_buffer[i];
                const auto estimated_value = state.get(output_ids[i]);
                penalty += (new_value - estimated_value) * (new_value - estimated_value);
            }
            break;
        }
        case EquationType::statefulNLEquation:
        {
            auto& eq = m_stateful_equations[eq_idx.index];
            auto& input_ids = eq.input_ids;
            auto input_buffer = std::span{input_temp_buffer}.subspan(0, input_ids.size());
            assert(input_buffer.size() == input_ids.size());
            for (size_t i = 0; i < input_buffer.size(); i++)
            {
                const auto val = state.get(input_ids[i]);
                input_buffer[i] = val;
            }

            auto& output_ids = eq.output_ids;
            auto output_buffer = std::span{ output_temp_buffer }.subspan(0, output_ids.size());

            auto res = eq.equation->Apply(input_buffer, output_buffer, m_current_time, eq.data);
            if (res == Status::error)
            {
                return tl::unexpected<std::string>{ eq.equation->GetLastError() };
            }
            assert(output_buffer.size() == output_ids.size());
            for (size_t i = 0; i < output_buffer.size(); i++)
            {
                const double new_value = output_buffer[i];
                const auto estimated_value = state.get(output_ids[i]);
                penalty += (new_value - estimated_value) * (new_value - estimated_value);
            }
            break;
        }
        case EquationType::BufferEquation:
        {
            const auto& eq = m_buffer_equations[eq_idx.index];
            const double new_value = state.get(eq.input_id);
            const auto estimated_value = state.get(eq.output_id);
            penalty += (new_value - estimated_value) * (new_value - estimated_value);
            break;
        }
        }
    }
    return std::sqrt(penalty);
}

opt::NLSolveResult opt::NLGraphSolver_impl::UpdateStateInternal(FlatMap& state)
{
    m_last_state_time = m_current_time;
    std::array<double, 20> input_temp_buffer;

    for (auto element : m_stateful_equations | boost::adaptors::indexed())
    {
        auto& eq = element.value();
        auto& input_ids = eq.input_ids;
        auto input_buffer = std::span{input_temp_buffer}.subspan(0, input_ids.size());
        assert(input_buffer.size() == input_ids.size());
        for (size_t i = 0; i < input_buffer.size(); i++)
        {
            const auto val = state.get(input_ids[i]);
            input_buffer[i] = val;
        }
        auto res = eq.equation->Update(input_buffer, m_current_time, eq.data);
        if (res == Status::error)
        {
            return tl::unexpected<std::string>{ eq.equation->GetLastError() };
        }
    }
    return std::monostate{};
}

std::vector<opt::NLStatefulEquationWrapper>& opt::NLGraphSolver_impl::GetStatefulEquations()
{
    return m_stateful_equations;
}

opt::NLGraphSolver_impl::NLGraphSolver_impl(std::vector<NLEquationWrapper> equations)
    :m_equations(std::move(equations))
{
}

opt::NLGraphSolver_impl::NLGraphSolver_impl()
{
}

void opt::NLGraphSolver_impl::Initialize()
{
    std::vector<int32_t> remaining_output_ids;

    for (const auto& item : m_equations)
    {
		auto&& range = item.output_ids;
        remaining_output_ids.insert(remaining_output_ids.end(), range.begin(), range.end());
    }
    for (const auto& item : m_stateful_equations)
    {
        auto&& range = item.output_ids;
        remaining_output_ids.insert(remaining_output_ids.end(), range.begin(), range.end());
    }
    for (const auto& item : m_buffer_equations)
    {
        auto&& id = item.output_id;
        remaining_output_ids.push_back(id);
    }

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
    m_optimizer.set_xtol_rel(1e-4);
    m_optimizer.set_xtol_abs(1e-6);
    m_optimizer.set_ftol_rel(1e-4);
    m_optimizer.set_ftol_abs(1e-6);
}

void opt::NLGraphSolver_impl::FillInitialSolveEqns(std::vector<int32_t>& remaining_output_ids)
{
    // khan's algorithm, we see which blocks have no unevaluated inputs, add them to inital functors, then see whether the connected blocks can be evaluated yet.
    struct BlockInputsCount
    {
        EquationIndex index;
        size_t count;
    };

    std::vector<BlockInputsCount> all_blocks;
    std::unordered_map<int32_t, std::vector<size_t>> edges; // map output_id -> all_blocks index
    std::queue<EquationIndex> blocks_to_process;

    for (size_t i = 0; i < m_equations.size(); i++)
    {
        all_blocks.push_back({ {i, EquationType::NLEquation}, 0 });
        auto&& inputs = m_equations[i].input_ids;
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
        all_blocks.push_back({ {i, EquationType::statefulNLEquation}, 0 });
        auto&& inputs = m_stateful_equations[i].input_ids;
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
    for (size_t i = 0; i < m_buffer_equations.size(); i++)
    {
        all_blocks.push_back({ {i, EquationType::BufferEquation}, 0 });
        auto&& input_id = m_buffer_equations[i].input_id;
        auto it = std::find(remaining_output_ids.begin(), remaining_output_ids.end(), input_id);
        if (it != remaining_output_ids.end())
        {
            all_blocks.back().count++;
        }
        edges[input_id].push_back(all_blocks.size() - 1);
        if (all_blocks.back().count == 0)
        {
            blocks_to_process.push(all_blocks.back().index);
        }
    }

    while (blocks_to_process.size())
    {
        auto index = blocks_to_process.front();
        blocks_to_process.pop();

        auto output_ids = [&]()->std::span<const int32_t>
            {
                switch (index.type)
                {
                case EquationType::NLEquation:
                {
                    return m_equations[index.index].output_ids;
                }
                case EquationType::statefulNLEquation:
                {
                    return m_stateful_equations[index.index].output_ids;
                }
                case EquationType::BufferEquation:
                {
                    return { &m_buffer_equations[index.index].output_id, 1 };
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

opt::NLSolveResult opt::NLGraphSolver_impl::EvalSpecificFunctors(FlatMap& state, const std::vector<EquationIndex>& indicies)
{
    std::array<double, 20> input_temp_buffer;
    std::array<double, 20> output_temp_buffer;
    for (const auto& eq_index : indicies)
    {
        switch (eq_index.type)
        {
        case EquationType::NLEquation:
        {
            auto& eq = m_equations[eq_index.index];
            auto& input_ids = eq.input_ids;
            auto input_buffer = std::span{input_temp_buffer}.subspan(0, input_ids.size());
            assert(input_buffer.size() == input_ids.size());
            for (size_t i = 0; i < input_buffer.size(); i++)
            {
                const auto val = state.get(input_ids[i]);
                input_buffer[i] = val;
            }

            auto& output_ids = eq.output_ids;
            auto output_buffer = std::span{output_temp_buffer}.subspan(0, output_ids.size());

            auto res = eq.equation->Apply(input_buffer, output_buffer);
            if (res == Status::error)
            {
                return tl::unexpected<std::string>{ eq.equation->GetLastError() };
            }
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
            auto& input_ids = eq.input_ids;
            auto input_buffer = std::span{input_temp_buffer}.subspan(0, input_ids.size());
            assert(input_buffer.size() == input_ids.size());
            for (size_t i = 0; i < input_buffer.size(); i++)
            {
                const auto val = state.get(input_ids[i]);
                input_buffer[i] = val;
            }
            auto& output_ids = eq.output_ids;
            auto output_buffer = std::span{ output_temp_buffer }.subspan(0, output_ids.size());
            
            auto res = eq.equation->Apply(input_buffer, output_buffer, m_current_time, eq.data);
            if (res == Status::error)
            {
                return tl::unexpected<std::string>{ eq.equation->GetLastError() };
            }
            assert(output_buffer.size() == output_ids.size());
            for (size_t i = 0; i < output_buffer.size(); i++)
            {
                const double new_value = output_buffer[i];
                state.modify(output_ids[i], new_value);
            }
            break;
        }
        case EquationType::BufferEquation:
        {
            const auto& eq = m_buffer_equations[eq_index.index];
            const double new_value = state.get(eq.input_id);
            state.modify(eq.output_id, new_value);
            break;
        }
        }
    }
    return std::monostate{};
}

void opt::NLGraphSolver_impl::FillInnerSolveEqns(std::vector<int32_t>& remaining_output_ids)
{
    struct BlockInputsCount
    {
        EquationIndex index;
        size_t count;
    };

    std::vector<BlockInputsCount> all_blocks;
    std::unordered_map<int32_t, std::vector<size_t>> edges; // map output_id -> all_blocks index

    for (size_t i = 0; i < m_equations.size(); i++)
    {
        all_blocks.push_back({ {i, EquationType::NLEquation}, 0 });
        auto&& inputs = m_equations[i].input_ids;
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
        all_blocks.push_back({ {i, EquationType::statefulNLEquation}, 0 });
        auto&& inputs = m_stateful_equations[i].input_ids;
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
    for (size_t i = 0; i < m_buffer_equations.size(); i++)
    {
        all_blocks.push_back({ {i, EquationType::BufferEquation}, 0 });
        auto&& input_id = m_buffer_equations[i].input_id;
        auto it = std::find(remaining_output_ids.begin(), remaining_output_ids.end(), input_id);
        if (it != remaining_output_ids.end())
        {
            all_blocks.back().count++;
        }
        edges[input_id].push_back(all_blocks.size() - 1);
    }

    auto block_has_remaining_id = [&](const EquationIndex& index) -> bool
        {
            switch (index.type)
            {
            case EquationType::NLEquation:
            {
                for (auto&& id : m_equations[index.index].output_ids)
                {
                    if (std::find(remaining_output_ids.begin(), remaining_output_ids.end(), id) != remaining_output_ids.end())
                    {
                        return true;
                    }
                }
                break;
            }
            case EquationType::statefulNLEquation:
            {
                for (auto&& id : m_stateful_equations[index.index].input_ids)
                {
                    if (std::find(remaining_output_ids.begin(), remaining_output_ids.end(), id) != remaining_output_ids.end())
                    {
                        return true;
                    }
                }
                break;
            }
            case EquationType::BufferEquation:
            {
                auto&& id = m_buffer_equations[index.index].output_id;
                if (std::find(remaining_output_ids.begin(), remaining_output_ids.end(), id) != remaining_output_ids.end())
                {
                    return true;
                }
                break;
            }
            }
            return false;
        };

    std::queue<EquationIndex> blocks_to_process;
    while (remaining_output_ids.size())
    {
        if (blocks_to_process.size())
        {
            auto index = blocks_to_process.front();
            blocks_to_process.pop();

            auto output_ids = [&]()->std::span<const int32_t>
                {
                    switch (index.type)
                    {
                    case EquationType::NLEquation:
                    {
                        return m_equations[index.index].output_ids;
                    }
                    case EquationType::statefulNLEquation:
                    {
                        return m_stateful_equations[index.index].output_ids;
                    }
                    case EquationType::BufferEquation:
                    {
                        return { &m_buffer_equations[index.index].output_id, 1 };
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
                if (!block_has_remaining_id(block.index))
                {
                    continue;
                }
                if (!max_block.has_value())
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
                                return m_equations[block_idx.index.index].output_ids.size();
                            }
                            case EquationType::statefulNLEquation:
                            {
                                return m_stateful_equations[block_idx.index.index].output_ids.size();
                            }
                            case EquationType::BufferEquation:
                            {
                                return 1;
                            }
                            }
                            return 0;
                        };
                    size_t first_output_ids_count = get_block_outputs_count(*max_block);
                    size_t second_output_ids_count = get_block_outputs_count(block);
                    if (second_output_ids_count > first_output_ids_count)
                    {
                        max_block = block;
                    }
                }
            }
            assert(max_block);

            // add it as estimated block
            m_estimated_eqns.push_back(max_block->index);
            all_blocks[max_block->index.index].count = 0;
            auto output_ids = [&]()->std::span<const int32_t>
                {
                    switch (max_block->index.type)
                    {
                    case EquationType::NLEquation:
                    {
                        return m_equations[max_block->index.index].output_ids;
                    }
                    case EquationType::statefulNLEquation:
                    {
                        return m_stateful_equations[max_block->index.index].output_ids;
                    }
                    case EquationType::BufferEquation:
                    {
                        return { &m_buffer_equations[max_block->index.index].output_id, 1 };
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

void opt::NLGraphSolver_impl::AddEquation(opt::NLEquationWrapper eq)
{
    m_equations.emplace_back(std::move(eq));
}

void opt::NLGraphSolver_impl::AddStatefulEquation(NLStatefulEquationWrapper eq)
{
    m_stateful_equations.push_back(std::move(eq));
}

void opt::NLGraphSolver_impl::AddBufferEq(BufferEquation eq)
{
    m_buffer_equations.push_back(std::move(eq));
}

static void OffloadSpecificIndicies(const opt::FlatMap& src, opt::FlatMap& dst, const std::vector<int32_t>& indicies)
{
    for (const auto& idx : indicies)
    {
        dst.modify(idx, src.get(idx));
    }
}

opt::NLSolveResult opt::NLGraphSolver_impl::Solve(FlatMap& state, const double& time)
{
    if ( 0 == m_estimated_output_ids.size() && 0 == m_initial_solve_output_ids.size())
    {
        return std::monostate{};
    }

    m_current_time = time;
    opt::FlatMap::sync(state, m_current_state);
    {
        auto res = EvalSpecificFunctors(m_current_state, m_initial_solve_eqns);
        if (!res)
        {
            return res;
        }
    }
    if (m_estimated_output_ids.size())
    {
        LoadMaptoVec(m_current_state, m_current_x);
        double min_value;
        [[maybe_unused]] nlopt::result result = m_optimizer.optimize(m_current_x, min_value);
        if (result == nlopt::FORCED_STOP)
        {
            return tl::unexpected{ m_last_error.value_or("") };
        }
        {
            auto res = EvalSpecificFunctors(m_current_state, m_inner_solve_eqns);
            if (!res)
            {
                return res;
            }
        }
        OffloadSpecificIndicies(m_current_state, state, m_initial_solve_output_ids);
        OffloadSpecificIndicies(m_current_state, state, m_inner_solve_output_ids);
        for (size_t idx = 0; idx < m_estimated_output_ids.size(); idx++)
        {
            state.modify(m_estimated_output_ids[idx], m_current_x[idx]);
        }
    }
    else
    {
        OffloadSpecificIndicies(m_current_state, state, m_initial_solve_output_ids);
    }
    //LoadDatatoMap(x, state);
    return std::monostate{};
}

opt::NLSolveResult opt::NLGraphSolver_impl::UpdateState(FlatMap& state, const double& time)
{
    if (0 == m_estimated_output_ids.size() && 0 == m_initial_solve_output_ids.size())
    {
        return std::monostate{};
    }

    m_current_time = time;
    opt::FlatMap::sync(state, m_current_state);

    return UpdateStateInternal(state);
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

opt::NLSolveResult opt::NLGraphSolver::Solve(FlatMap& state, const double& time)
{
    return m_impl->Solve(state, time);
}

opt::NLSolveResult opt::NLGraphSolver::UpdateState(FlatMap& state, const double& time)
{
    return m_impl->UpdateState(state, time);
}

void opt::NLGraphSolver::AddEquation(NLEquationWrapper eq)
{
    m_impl->AddEquation(std::move(eq));
}

void opt::NLGraphSolver::AddStatefulEquation(NLStatefulEquationWrapper eq)
{
    m_impl->AddStatefulEquation(std::move(eq));
}

void opt::NLGraphSolver::AddBufferEquation(BufferEquation eq)
{
    m_impl->AddBufferEq(std::move(eq));
}

std::vector<opt::NLStatefulEquationWrapper>& opt::NLGraphSolver::GetStatefulEquations()
{
    return m_impl->GetStatefulEquations();
}
