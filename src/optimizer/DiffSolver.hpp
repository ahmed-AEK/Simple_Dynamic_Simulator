#pragma once

#include "optimizer/DiffEquation.hpp"
#include "optimizer/flatmap.hpp"
#include <memory>

namespace opt
{

enum class StepResult
{
    Success,
    Fail,
    ReachedEnd,
};

class DiffSolver_impl;

class DiffSolver
{
public:
    DiffSolver();
    DiffSolver(DiffSolver&&) noexcept;
    DiffSolver& operator=(DiffSolver&&) noexcept;
    ~DiffSolver();
    void AddEquation(DiffEquation eq);
    void Initialize(double start_time, double end_time);
    StepResult Step(opt::FlatMap& state);
    [[nodiscard]] double GetStartTime() const;
    [[nodiscard]] double GetEndTime() const;
    [[nodiscard]] double GetCurrentTime() const;
    void SetPreprocessor(std::function<void(opt::FlatMap&, const double&)> preprocessor);
    void SetPostprocessor(std::function<void(opt::FlatMap&, const double&)> postprocessor);
    void ApplyPreprocessor(opt::FlatMap& state, const double t);
    void ApplyPostProcessor(opt::FlatMap& state, const double t);
    void SetMaxStep(double step_size);
    void SetNextEventTime(std::optional<double> t);
private:
    std::unique_ptr<DiffSolver_impl> m_impl;
};
}