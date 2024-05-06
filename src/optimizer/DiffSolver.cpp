#include "DiffSolver.hpp"
#include "cassert"
#include "toolgui/NodeMacros.h"
#include "boost/numeric/odeint/stepper/controlled_step_result.hpp"

class DiffSystem {

    opt::DiffSolver& m_solver;
public:
    DiffSystem(opt::DiffSolver& solver) : m_solver(solver) { }

    void operator() (const opt::DiffSolver::state_type& x, opt::DiffSolver::state_type& dxdt, const double t)
    {
        m_solver.StepInternal(x, dxdt, t);
    }
};

opt::DiffSolver::DiffSolver()
{

}

opt::DiffSolver::DiffSolver(std::vector<DiffEquation> equations)
: m_equations(std::move(equations))
{

}
void opt::DiffSolver::AddEquation(DiffEquation eq)
{
    m_equations.push_back(std::move(eq));
}

void opt::DiffSolver::Initialize(double start_time, double end_time)
{
    assert(end_time > start_time);
    m_start_time = start_time;
    m_current_time = start_time;
    m_end_time = end_time;
    m_output_ids.clear();
    for (const auto& item : m_equations)
    {
        for (const auto& output_id : item.get_output_ids())
        {
            m_output_ids.insert(output_id);
        }
    }
    m_current_x.reserve(m_output_ids.size());
    m_current_dxdt.reserve(m_output_ids.size());
}

void opt::DiffSolver::LoadDatatoMap(std::span<const double> x, FlatMap& state)
{
    assert(x.size() == m_output_ids.size());
    for (size_t i = 0; i < x.size(); i++)
    {
        state.modify(*(m_output_ids.begin()+ i), x[i]);
    }
}

void opt::DiffSolver::LoadMaptoVec(FlatMap& state, std::vector<double>& target)
{
    auto state_data = state.data();
    std::vector<double> output;
    output.reserve(state_data.size());
    target.clear();
    for (size_t i = 0; i < m_output_ids.size(); i++)
    {
        target.push_back(state.get(*(m_output_ids.begin()+i)));
    }
}

opt::StepResult opt::DiffSolver::Step(opt::FlatMap& state)
{
    opt::FlatMap::sync(state, m_current_state);
    LoadMaptoVec(state, m_current_x);

    auto diffsystem = DiffSystem(*this);
    auto result = boost::numeric::odeint::controlled_step_result::fail;
    if (m_last_dt + m_current_time > m_end_time)
    {
        m_last_dt = m_end_time - m_current_time;
    }

    while (result != boost::numeric::odeint::controlled_step_result::success)
    {
        result = m_stepper.try_step(diffsystem, m_current_x, m_current_time, m_last_dt);
    }
    LoadDatatoMap(m_current_x, state);
    if (m_current_time >= m_end_time)
    {
        m_current_time = m_end_time;
        return opt::StepResult::ReachedEnd;
    }
    return opt::StepResult::Success;
}

void opt::DiffSolver::StepInternal(const opt::DiffSolver::state_type& x, opt::DiffSolver::state_type& dxdt, const double t)
{
    LoadDatatoMap(x, m_current_state);
    if (m_preprocessor)
    {
        m_preprocessor(m_current_state, t);
    }
    
    for (auto& eq : m_equations)
    {
        auto input_buffer = eq.get_input_buffer();
        auto input_ids = eq.get_input_ids();
        assert(input_buffer.size() == input_ids.size());
        for (size_t i = 0; i < input_buffer.size(); i++)
        {
            const auto val = m_current_state.get(input_ids[i]);
            input_buffer[i] = val;
        }
        eq.Apply(t);
        auto output_buffer = eq.get_output_buffer();
        auto output_ids = eq.get_output_ids();
        assert(output_buffer.size() == output_ids.size());
        for (size_t i = 0; i < output_buffer.size(); i++)
        {
            const auto it = m_output_ids.find(output_ids[i]);
            const double new_value = output_buffer[i];
            dxdt[std::distance(m_output_ids.begin(), it)] = new_value;
        }
    }
}

void opt::DiffSolver::SetPreprocessor(std::function<void(opt::FlatMap&, const double&)> preprocessor)
{
    m_preprocessor = std::move(preprocessor);
}
