#pragma once

#include "optimizer/DiffEquation.hpp"
#include "optimizer/flatmap.hpp"
#include <memory>
#include "optimizer/NLGraphSolver.hpp"

namespace opt
{

enum class StepEnd
{
    Success,
    ReachedEnd,
};

using StepResult = tl::expected<StepEnd, std::string>;

class DiffSolver_impl;

class DiffSolver
{
public:
    DiffSolver();
    DiffSolver(DiffSolver&&) noexcept;
    DiffSolver& operator=(DiffSolver&&) noexcept;
    ~DiffSolver();
    void AddEquation(DiffEquationWrapper eq);
    void Initialize(double start_time, double end_time);
    StepResult Step(opt::FlatMap& state);
    [[nodiscard]] double GetStartTime() const;
    [[nodiscard]] double GetEndTime() const;
    [[nodiscard]] double GetCurrentTime() const;
    void SetPreprocessor(std::function<NLSolveResult(opt::FlatMap&, const double&)> preprocessor);
    void SetPostprocessor(std::function<NLSolveResult(opt::FlatMap&, const double&)> postprocessor);
    [[nodiscard]] opt::NLSolveResult ApplyPreprocessor(opt::FlatMap& state, const double t);
    [[nodiscard]] opt::NLSolveResult ApplyPostProcessor(opt::FlatMap& state, const double t);
    void SetMaxStep(double step_size);
    void SetNextEventTime(std::optional<double> t);
    void SetCurrentTime(const double& t) const;
    void InterpolateAt(FlatMap& state, const double& t);

    void GetOutputIndicies(std::vector<int32_t>& indicies) const;
private:
    std::unique_ptr<DiffSolver_impl> m_impl;
};
}