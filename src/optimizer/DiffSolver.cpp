#include "DiffSolver.hpp"

#include "boost/numeric/odeint/stepper/controlled_step_result.hpp"
#include "optimizer/DiffSolver_private.hpp"

class stop_diff_step_error : public std::exception
{
public:
    stop_diff_step_error(std::string error) : m_error_message{ std::move(error) } {}
    const char* what() const noexcept override
    {
        return m_error_message.c_str();
    }
    std::string& get_message() { return m_error_message; }
private:
    std::string m_error_message;
};

class DiffSystem {

    opt::DiffSolver_impl& m_solver;
public:
    explicit DiffSystem(opt::DiffSolver_impl& solver) : m_solver(solver) { }

    void operator() (const opt::DiffSolver_impl::state_type& x, opt::DiffSolver_impl::state_type& dxdt, const double t)
    {
        auto res = m_solver.StepInternal(x, dxdt, t);
        if (!res)
        {
            throw stop_diff_step_error{ std::move(res.error()) };
        }
    }
};

opt::DiffSolver_impl::DiffSolver_impl()
{

}

opt::DiffSolver_impl::DiffSolver_impl(std::vector<DiffEquationWrapper> equations)
: m_equations(std::move(equations))
{

}
void opt::DiffSolver_impl::AddEquation(DiffEquationWrapper eq)
{
    m_equations.push_back(std::move(eq));
}

void opt::DiffSolver_impl::Initialize(double start_time, double end_time)
{
    assert(end_time > start_time);
    m_start_time = start_time;
    m_current_time = start_time;
    m_end_time = end_time;
    m_output_ids.clear();
    for (const auto& item : m_equations)
    {
        for (const auto& output_id : item.output_ids)
        {
            m_output_ids.insert(output_id);
        }
    }
    m_current_x.reserve(m_output_ids.size());
    m_current_dxdt.reserve(m_output_ids.size());
    m_current_interpolation_x.resize(m_output_ids.size());
}

void opt::DiffSolver_impl::LoadDatatoMap(std::span<const double> x, FlatMap& state)
{
    assert(x.size() == m_output_ids.size());
    for (size_t i = 0; i < x.size(); i++)
    {
        state.modify(*(m_output_ids.begin()+ i), x[i]);
    }
}

void opt::DiffSolver_impl::LoadMaptoVec(const FlatMap& state, std::vector<double>& target)
{
    target.clear();
    target.reserve(m_output_ids.size());
    for (size_t i = 0; i < m_output_ids.size(); i++)
    {
        target.push_back(state.get(*(m_output_ids.begin()+i)));
    }
}

opt::StepResult opt::DiffSolver_impl::Step(opt::FlatMap& state)
{

    opt::FlatMap::sync(state, m_current_state);
    LoadMaptoVec(state, m_current_x);

    auto diffsystem = DiffSystem(*this);
    auto result = boost::numeric::odeint::controlled_step_result::fail;

    if (m_last_dt > m_max_step)
    {
        // correct for max step size
        m_last_dt = m_max_step;
    }

    if (m_next_event_time && (m_current_time + m_last_dt > *m_next_event_time))
    {
        // correct for next event time
        m_last_dt = *m_next_event_time - m_current_time;
    }

    double last_dt_saved = m_last_dt;

    while (result != boost::numeric::odeint::controlled_step_result::success)
    {
        if (m_equations.size())
        {
            m_stepper.initialize(m_current_x, m_current_time, m_last_dt);
            try
            {
                auto [start_time, end_time] = m_stepper.do_step(diffsystem);
                m_current_x = m_stepper.current_state();
                m_last_dt = m_stepper.current_time_step();
                m_current_time = end_time;
                m_interpolation_start_time = start_time;
                m_interpolation_end_time = end_time;
                result = boost::numeric::odeint::success;
            }
            catch (stop_diff_step_error& e)
            {
                return tl::unexpected{ std::move(e.get_message()) };
            }
        }
        else
        {
            m_current_time += m_last_dt;
            if (m_last_dt < m_max_step)
            {
                m_last_dt = m_max_step;
            }
            result = boost::numeric::odeint::controlled_step_result::success;
        }
    }
    LoadDatatoMap(m_current_x, state);

    if (m_next_event_time && ((*m_next_event_time - m_current_time) / last_dt_saved) < 1e-4)
    {
        // to avoid floating point errors, if time to next event is within 0.01% of the dt, then we jump to it
        m_current_time = *m_next_event_time;
    }

    if (m_current_time >= m_end_time)
    {
        m_current_time = m_end_time;
        return opt::StepEnd::ReachedEnd;
    }
    return opt::StepEnd::Success;
}

opt::NLSolveResult opt::DiffSolver_impl::StepInternal(const opt::DiffSolver_impl::state_type& x, opt::DiffSolver_impl::state_type& dxdt, const double t)
{
    LoadDatatoMap(x, m_current_state);

    {
        auto pre_result = ApplyPreprocessor(m_current_state, t);
        if (!pre_result)
        {
            return pre_result;
        }
    }
    
    std::array<double, 20> input_temp_buff;
    std::array<double, 20> ouput_temp_buff;

    for (auto& eq : m_equations)
    {
        auto& input_ids = eq.input_ids;
        auto input_buffer = std::span{input_temp_buff}.subspan(0, input_ids.size());
        assert(input_buffer.size() == input_ids.size());
        for (size_t i = 0; i < input_buffer.size(); i++)
        {
            const auto val = m_current_state.get(input_ids[i]);
            input_buffer[i] = val;
        }

        auto& output_ids = eq.output_ids;
        auto output_buffer = std::span{ ouput_temp_buff }.subspan(0, output_ids.size());
        

        {
            auto res = eq.equation->Apply(input_buffer, output_buffer, t);
            if (res == Status::error)
            {
                return tl::unexpected<std::string>{ eq.equation->GetLastError() };
            }
        }
        assert(output_buffer.size() == output_ids.size());
        for (size_t i = 0; i < output_buffer.size(); i++)
        {
            const auto it = m_output_ids.find(output_ids[i]);
            const double new_value = output_buffer[i];
            dxdt[std::distance(m_output_ids.begin(), it)] = new_value;
        }
    }
    return std::monostate{};
}

void opt::DiffSolver_impl::SetPreprocessor(std::function<NLSolveResult(opt::FlatMap&, const double&)> preprocessor)
{
    m_preprocessor = std::move(preprocessor);
}

void opt::DiffSolver_impl::SetPostprocessor(std::function<NLSolveResult(opt::FlatMap&, const double&)> postprocessor)
{
    m_postprocessor = std::move(postprocessor);
}

opt::NLSolveResult opt::DiffSolver_impl::ApplyPreprocessor(opt::FlatMap& state, const double t)
{
    if (m_preprocessor)
    {
        return m_preprocessor(state, t);
    }
    return std::monostate{};
}

opt::NLSolveResult opt::DiffSolver_impl::ApplyPostProcessor(opt::FlatMap& state, const double t)
{
    if (m_postprocessor)
    {
        return m_postprocessor(state, t);
    }
    return std::monostate{};
}

void opt::DiffSolver_impl::SetMaxStep(double step_size)
{
    m_max_step = step_size;
}

void opt::DiffSolver_impl::SetNextEventTime(std::optional<double> t)
{
    m_next_event_time = t;
}

void opt::DiffSolver_impl::SetCurrentTime(const double& t)
{
    m_current_time = t;
}

void opt::DiffSolver_impl::InterpolateAt(FlatMap& state, const double& t)
{
    if (!m_equations.size())
    {
        return;
    }
    assert(t >= m_interpolation_start_time);
    assert(t <= m_interpolation_end_time);

    m_stepper.calc_state(t, m_current_interpolation_x);
    LoadDatatoMap(m_current_interpolation_x, state);
}

void opt::DiffSolver_impl::GetOutputIndicies(std::vector<int32_t>& indicies) const
{
    for (const auto& eq : m_equations)
    {
        auto&& output_ids = eq.output_ids;
        indicies.insert(indicies.end(), output_ids.begin(), output_ids.end());
    }
}

opt::DiffSolver::DiffSolver()
    :m_impl(std::make_unique<DiffSolver_impl>())
{
}

opt::DiffSolver::DiffSolver(DiffSolver&&) noexcept = default;

opt::DiffSolver& opt::DiffSolver::operator=(DiffSolver&&) noexcept = default;

opt::DiffSolver::~DiffSolver() = default;

void opt::DiffSolver::AddEquation(DiffEquationWrapper eq)
{
    m_impl->AddEquation(std::move(eq));
}

void opt::DiffSolver::Initialize(double start_time, double end_time)
{
    m_impl->Initialize(start_time, end_time);
}

opt::StepResult opt::DiffSolver::Step(opt::FlatMap& state)
{
    return m_impl->Step(state);
}

double opt::DiffSolver::GetStartTime() const
{
    return m_impl->GetStartTime();
}

double opt::DiffSolver::GetEndTime() const
{
    return m_impl->GetEndTime();
}

double opt::DiffSolver::GetCurrentTime() const
{
    return m_impl->GetCurrentTime();
}

void opt::DiffSolver::SetPreprocessor(std::function<NLSolveResult(opt::FlatMap&, const double&)> preprocessor)
{
    m_impl->SetPreprocessor(std::move(preprocessor));
}

void opt::DiffSolver::SetPostprocessor(std::function<NLSolveResult(opt::FlatMap&, const double&)> postprocessor)
{
    m_impl->SetPostprocessor(std::move(postprocessor));
}

opt::NLSolveResult opt::DiffSolver::ApplyPreprocessor(opt::FlatMap& state, const double t)
{
    return m_impl->ApplyPreprocessor(state, t);
}

opt::NLSolveResult  opt::DiffSolver::ApplyPostProcessor(opt::FlatMap& state, const double t)
{
    return m_impl->ApplyPostProcessor(state, t);
}

void opt::DiffSolver::SetMaxStep(double step_size)
{
    m_impl->SetMaxStep(step_size);
}

void opt::DiffSolver::SetNextEventTime(std::optional<double> t)
{
    m_impl->SetNextEventTime(t);
}

void opt::DiffSolver::SetCurrentTime(const double& t) const
{
    m_impl->SetCurrentTime(t);
}

void opt::DiffSolver::InterpolateAt(FlatMap& state, const double& t)
{
    m_impl->InterpolateAt(state, t);
}

void opt::DiffSolver::GetOutputIndicies(std::vector<int32_t>& indicies) const
{
    m_impl->GetOutputIndicies(indicies);
}





