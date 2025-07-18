#pragma once

#include "optimizer/DiffEquation.hpp"
#include "optimizer/flatmap.hpp"

#include "boost/numeric/odeint/stepper/runge_kutta_dopri5.hpp"
#include "boost/numeric/odeint/stepper/controlled_runge_kutta.hpp"
#include "boost/numeric/odeint/stepper/dense_output_runge_kutta.hpp"
#include "boost/container/flat_set.hpp"
#include "optimizer/DiffSolver.hpp"

namespace opt
{

class DiffSolver_impl
{
public:
    using state_type = std::vector<double>;
    using error_stepper = boost::numeric::odeint::runge_kutta_dopri5<state_type>;
    using controlled_stepper = boost::numeric::odeint::controlled_runge_kutta<error_stepper>;
    using dense_stepper = boost::numeric::odeint::dense_output_runge_kutta<controlled_stepper>;

    using time_type = controlled_stepper::time_type;

    explicit DiffSolver_impl(std::vector<DiffEquationWrapper> eq);
    DiffSolver_impl();
    void AddEquation(DiffEquationWrapper eq);
    void Initialize(double start_time, double end_time);
    StepResult Step(opt::FlatMap& state);
    [[nodiscard]] constexpr double GetStartTime() const { return m_start_time; }
    [[nodiscard]] constexpr double GetEndTime() const { return m_end_time; }
    [[nodiscard]] constexpr double GetCurrentTime() const { return m_current_time; }
    [[nodiscard]] NLSolveResult StepInternal(const opt::DiffSolver_impl::state_type& x, opt::DiffSolver_impl::state_type& dxdt, const double t);
    void SetPreprocessor(std::function<NLSolveResult(opt::FlatMap&, const double&)> preprocessor);
    void SetPostprocessor(std::function<NLSolveResult(opt::FlatMap&, const double&)> postprocessor);
    [[nodiscard]] NLSolveResult ApplyPreprocessor(opt::FlatMap& state, const double t);
    [[nodiscard]] NLSolveResult ApplyPostProcessor(opt::FlatMap& state, const double t);
    void SetMaxStep(double step_size);
    void SetNextEventTime(std::optional<double> t);
    void SetCurrentTime(const double& t);
    void InterpolateAt(FlatMap& state, const double& t);

    void GetOutputIndicies(std::vector<int32_t>& indicies) const;
protected:
    void LoadDatatoMap(std::span<const double> x, FlatMap& state);
    void LoadMaptoVec(const FlatMap& state, std::vector<double>& target);
private:

    // equations
    std::vector<DiffEquationWrapper> m_equations;

    // per step state
    dense_stepper m_stepper{ controlled_stepper{
        boost::numeric::odeint::default_error_checker< double ,
        boost::numeric::odeint::range_algebra , boost::numeric::odeint::default_operations>(1e-4 , 1e-4, 1, 1)
    } };
    controlled_stepper::state_type m_current_x;
    controlled_stepper::state_type m_current_dxdt;
    controlled_stepper::state_type m_current_interpolation_x;
    controlled_stepper::time_type m_current_time = 0;
    controlled_stepper::time_type m_last_dt = 0.1;
    double m_interpolation_start_time = 0;
    double m_interpolation_end_time = 0;
    std::optional<double> m_next_event_time;
    opt::FlatMap m_current_state;

    // solver configuration
    boost::container::flat_set<int32_t> m_output_ids;
    controlled_stepper::time_type m_start_time = 0;
    controlled_stepper::time_type m_end_time = 0;
    controlled_stepper::time_type m_max_step = 0.1;
    std::function<NLSolveResult(opt::FlatMap&, const double&)> m_preprocessor;
    std::function<NLSolveResult(opt::FlatMap&, const double&)> m_postprocessor;
};
}