 
#include "gtest/gtest.h"
#include "optimizer/NLDiffSolver.hpp"
#include "toolgui/NodeMacros.h"

TEST(testNLDiffSolver, testConstructor)
{
    opt::NLDiffSolver solver;
}

TEST(testNLDiffSolver, testInitialize)
{
    opt::NLDiffSolver solver;
    solver.Initialize(0, 10);

    EXPECT_EQ(solver.GetStartTime(), 0);
    EXPECT_EQ(solver.GetEndTime(), 10);
}

TEST(testNLDiffSolver, testStep_time_advances)
{
    opt::DiffEquation eq({ 0 }, { 1 }, [](auto in, auto out, auto t) { return out[0] = in[0]; UNUSED_PARAM(t);});
    opt::NLDiffSolver solver;
    solver.AddDiffEquation(std::move(eq));
    solver.Initialize(0, 2);
    opt::FlatMap state(2);
    state.modify(0,1);
    state.modify(1,0);
    double current_time = 0;

    solver.CalculateInitialConditions(state);
    while (solver.Step(state) != opt::StepResult::ReachedEnd)
    {
        EXPECT_GT(solver.GetCurrentTime(), current_time);
        current_time = solver.GetCurrentTime();
    }
    EXPECT_EQ(solver.GetCurrentTime(), 2);
    EXPECT_NEAR(state.get(1), 2, 2e-3);
}


TEST(testNLDiffSolver, testNLEquations_simple)
{
    opt::NLEquation eq({ 0 }, { 1 }, [](auto in, auto out) -> void { out[0] = in[0] * 2; });
    opt::NLDiffSolver solver;
    solver.AddNLEquation(std::move(eq));
    solver.Initialize(0, 2);
    opt::FlatMap state(2);
    state.modify(0,1);
    state.modify(1,0);
    solver.CalculateInitialConditions(state);

    EXPECT_NEAR(state.get(1), 2, 2e-3);
}


TEST(testNLDiffSolver, testNLEquations_simple_observer)
{
    opt::NLEquation eq({ 0 }, { 1 }, [](auto in, auto out) -> void { out[0] = in[0] * 2; });
    opt::NLDiffSolver solver;

    std::vector<std::vector<double>> observer_log;
    opt::Observer observer({ 0, 1 }, [&](auto inputs, const auto& t)
        {
            observer_log.push_back({ inputs[0], inputs[1], t });
        });
    solver.AddObserver(std::move(observer));

    solver.AddNLEquation(std::move(eq));
    solver.Initialize(0, 2);
    opt::FlatMap state(2);
    state.modify(0, 1);
    state.modify(1, 0);
    solver.CalculateInitialConditions(state);

    EXPECT_NEAR(state.get(1), 2, 2e-3);
    EXPECT_EQ(state.get(0), observer_log[0][0]);
    EXPECT_EQ(state.get(1), observer_log[0][1]);
    EXPECT_EQ(0, observer_log[0][2]);
}
TEST(testNLDiffSolver, testNLDiffEquations_1d1n)
{
    opt::DiffEquation eq({ 1 }, { 2 }, [](auto in, auto out, auto t) -> void { out[0] = in[0]; UNUSED_PARAM(t);});
    opt::NLEquation eq2({ 0 }, { 1 }, [](auto in, auto out) -> void { out[0] = in[0] * 2; });
    opt::NLDiffSolver solver;
    solver.AddDiffEquation(std::move(eq));
    solver.AddNLEquation(std::move(eq2));
    solver.Initialize(0, 2);
    opt::FlatMap state(3);
    state.modify(0,1);
    state.modify(1,0);
    state.modify(2,0);
    double current_time = 0;

    solver.CalculateInitialConditions(state);

    while (solver.Step(state) != opt::StepResult::ReachedEnd)
    {
        EXPECT_GT(solver.GetCurrentTime(), current_time);
        current_time = solver.GetCurrentTime();
    }

    EXPECT_EQ(solver.GetCurrentTime(), 2);
    EXPECT_NEAR(state.get(1), 2, 2e-3);
    EXPECT_NEAR(state.get(2), 4, 2e-3);
}

TEST(testNLDiffSolver, testNLDiffEquations_1d1n_observer)
{
    opt::DiffEquation eq({ 1 }, { 2 }, [](auto in, auto out, auto t) -> void { out[0] = in[0]; UNUSED_PARAM(t); });
    opt::NLEquation eq2({ 0 }, { 1 }, [](auto in, auto out) -> void { out[0] = in[0] * 2; });
    opt::NLDiffSolver solver;

    std::vector<std::vector<double>> observer_log;
    opt::Observer observer({ 0, 1, 2 }, [&](auto inputs, const auto& t)
        {
            observer_log.push_back({ inputs[0], inputs[1], inputs[2], t});
        });
    solver.AddObserver(std::move(observer));

    solver.AddDiffEquation(std::move(eq));
    solver.AddNLEquation(std::move(eq2));
    solver.Initialize(0, 2);
    opt::FlatMap state(3);
    state.modify(0, 1);
    state.modify(1, 0);
    state.modify(2, 0);
    double current_time = 0;

    solver.CalculateInitialConditions(state);

    while (solver.Step(state) != opt::StepResult::ReachedEnd)
    {
        EXPECT_GT(solver.GetCurrentTime(), current_time);
        current_time = solver.GetCurrentTime();
    }

    EXPECT_EQ(solver.GetCurrentTime(), 2);
    EXPECT_NEAR(state.get(1), 2, 2e-3);
    EXPECT_NEAR(state.get(2), 4, 2e-3);

    const auto& last_log = observer_log.back();
    EXPECT_EQ(state.get(0), last_log[0]);
    EXPECT_EQ(state.get(1), last_log[1]);
    EXPECT_EQ(state.get(2), last_log[2]);
    EXPECT_EQ(2, last_log[3]);

}

TEST(testNLDiffSolver, testNLDiffEquations_1d2n)
{
    opt::DiffEquation eq({ 1 }, { 2 }, [](auto in, auto out, auto t) -> void { out[0] = in[0]; UNUSED_PARAM(t);});
    opt::NLEquation eq2({ 0 }, { 1 }, [](auto in, auto out) -> void { out[0] = in[0] * 2; });
    opt::NLEquation eq3({ 2 }, { 3 }, [](auto in, auto out) -> void { out[0] = in[0] * 2; });
    opt::NLDiffSolver solver;
    solver.AddDiffEquation(std::move(eq));
    solver.AddNLEquation(std::move(eq2));
    solver.AddNLEquation(std::move(eq3));
    solver.Initialize(0, 2);
    opt::FlatMap state(4);
    state.modify(0,1);
    state.modify(1,0);
    state.modify(2,0);
    state.modify(3,0);
    double current_time = 0;

    solver.CalculateInitialConditions(state);
    while (solver.Step(state) != opt::StepResult::ReachedEnd)
    {
        EXPECT_GT(solver.GetCurrentTime(), current_time);
        current_time = solver.GetCurrentTime();
    }

    EXPECT_EQ(solver.GetCurrentTime(), 2);
    EXPECT_NEAR(state.get(1), 2, 2e-3);
    EXPECT_NEAR(state.get(2), 4, 4e-3);
    EXPECT_NEAR(state.get(3), 8, 8e-3);
}