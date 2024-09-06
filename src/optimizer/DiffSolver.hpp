#pragma once

#include "optimizer/DiffEquation.hpp"
#include "optimizer/flatmap.hpp"

#include "boost/numeric/odeint/stepper/runge_kutta_cash_karp54.hpp"
#include "boost/numeric/odeint/stepper/controlled_runge_kutta.hpp"
#include "boost/container/flat_set.hpp"

namespace opt
{

enum class StepResult
{
    Success,
    Fail,
    ReachedEnd,
};

class DiffSolver
{
public:
    using state_type = std::vector<double>;
    using error_stepper = boost::numeric::odeint::runge_kutta_cash_karp54<state_type>;
    using controlled_stepper = boost::numeric::odeint::controlled_runge_kutta<error_stepper>;

    using time_type = controlled_stepper::time_type;

    explicit DiffSolver(std::vector<DiffEquation> eq);
    DiffSolver();
    void AddEquation(DiffEquation eq);
    void Initialize(double start_time, double end_time);
    StepResult Step(opt::FlatMap& state);
    [[nodiscard]] constexpr double GetStartTime() const {return m_start_time;}
    [[nodiscard]] constexpr double GetEndTime() const {return m_end_time;}
    [[nodiscard]] constexpr double GetCurrentTime() const {return m_current_time;}
    void StepInternal(const opt::DiffSolver::state_type& x, opt::DiffSolver::state_type& dxdt, const double t);
    void SetPreprocessor(std::function<void(opt::FlatMap&, const double&)> preprocessor);
    void SetPostprocessor(std::function<void(opt::FlatMap&, const double&)> postprocessor);
    void ApplyPreprocessor(opt::FlatMap& state, const double t);
    void ApplyPostProcessor(opt::FlatMap& state, const double t);
protected:
    void LoadDatatoMap(std::span<const double> x, FlatMap& state);
    void LoadMaptoVec(FlatMap& state, std::vector<double>& target);
private:
    controlled_stepper m_stepper
    {
        boost::numeric::odeint::default_error_checker< double , 
        boost::numeric::odeint::range_algebra , boost::numeric::odeint::default_operations 
        >(1e-4 , 1e-4, 1, 1)
    };
    std::vector<DiffEquation> m_equations;
    boost::container::flat_set<int64_t> m_output_ids;
    controlled_stepper::state_type m_current_x;
    controlled_stepper::state_type m_current_dxdt;
    opt::FlatMap m_current_state;
    controlled_stepper::time_type m_start_time = 0;
    controlled_stepper::time_type m_end_time = 0;
    controlled_stepper::time_type m_current_time = 0;
    controlled_stepper::time_type m_last_dt = 0.1;
    controlled_stepper::time_type m_max_step = 0.1;
    std::function<void(opt::FlatMap&, const double&)> m_preprocessor;
    std::function<void(opt::FlatMap&, const double&)> m_postprocessor;
};
}