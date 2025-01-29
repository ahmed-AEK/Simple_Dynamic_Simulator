

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

TEST(testNLDiffSolver, testNLDiffEquations_multiply_diff)
{

    struct DerivativeStateTest
    {
        double last_input;
        double last_input_time;
        double last_out;
    };

    auto s1 = opt::SourceEq{ {0} ,[](std::span<double> out, const double& t, opt::SourceEq&) { out[0] = std::sin(2 * std::numbers::pi * t); } };
    auto s2 = opt::SourceEq{ {1} ,[](std::span<double> out, const double& t, opt::SourceEq&) { out[0] = std::sin(2 * std::numbers::pi * t); } };
    auto mul = opt::NLEquation{ {0,1}, {2}, [](std::span<const double> in, std::span<double> out) {out[0] = in[0] * in[1]; } };
    auto diff = opt::NLStatefulEquation{
        {2},
        {3},
        opt::NLStatefulEquation::NLStatefulFunctor{[](std::span<const double> in, std::span<double> out, const double t, const opt::NLStatefulEquation& eq)
        {
            const auto& old_state = eq.GetState();
            if (old_state.contains<DerivativeStateTest>())
            {
                const auto& state = old_state.get<const DerivativeStateTest>();
                if (t == state.last_input_time)
                {
                    out[0] = state.last_out;
                }
                else
                {
                    out[0] = (in[0] - state.last_input) / (t - state.last_input_time);
                }
            }
            else
            {
                out[0] = 0;
            }
        }},
        opt::NLStatefulEquation::NLStatefulUpdateFunctor{[](std::span<const double> in, const double t, opt::NLStatefulEquation& eq)
        {
            auto& old_state = eq.GetState();
            if (old_state.contains<DerivativeStateTest>())
            {
                auto& state = old_state.get<DerivativeStateTest>();
                state.last_out = (in[0] - state.last_input) / (t - state.last_input_time);;
                state.last_input = in[0];
                state.last_input_time = t;
            }
            else
            {
                old_state = opt::FatAny{DerivativeStateTest{in[0], t, 0}};
            }
        }
    }
    };

    opt::NLDiffSolver solver;
    solver.AddSource(std::move(s1));
    solver.AddSource(std::move(s2));
    solver.AddNLEquation(std::move(mul));
    solver.AddNLStatefulEquation(std::move(diff));
    solver.Initialize(0, 10);
    solver.SetMaxStep(0.01);
    opt::FlatMap state(4);
    state.modify(0, 1);
    state.modify(1, 0);
    state.modify(2, 0);
    state.modify(3, 0);
    double current_time = 0;

    solver.CalculateInitialConditions(state);
    while (solver.Step(state) != opt::StepResult::ReachedEnd)
    {
        EXPECT_GT(solver.GetCurrentTime(), current_time);
        current_time = solver.GetCurrentTime();
    }

    EXPECT_NEAR(10, solver.GetCurrentTime(), 1e-3);
}

TEST(testNLDiffSolver, test_SourceEvent)
{
    bool event1_set = false;
    double event1_time = 0;
    auto s1 = opt::SourceEq{ {0} ,[&](std::span<double> out, const double&, opt::SourceEq&) {
            out[0] = 0;
        },
        [&](const double& t, opt::SourceEq&)
        {
            event1_set = true;
            event1_time = t;
        }
    };
    s1.GetEvent() = opt::SourceEq::SourceEvent{ 0.2,false };

    bool event2_set = false;
    double event2_time = 0;
    auto s2 = opt::SourceEq{ {1} ,[&](std::span<double> out, const double&, opt::SourceEq&) {
            out[0] = 0;
        },
        [&](const double& t, opt::SourceEq&)
        {
            event2_set = true;
            event2_time = t;
        }
    };
    s2.GetEvent() = opt::SourceEq::SourceEvent{ 0.201,false };

    opt::NLDiffSolver solver;
    solver.AddSource(std::move(s1));
    solver.AddSource(std::move(s2));

    opt::FlatMap state(2);
    double current_time = 0;

    solver.Initialize(0, 0.5);
    solver.SetMaxStep(0.01);
    solver.CalculateInitialConditions(state);
    while (solver.Step(state) != opt::StepResult::ReachedEnd)
    {
        EXPECT_GT(solver.GetCurrentTime(), current_time);
        current_time = solver.GetCurrentTime();
    }

    EXPECT_EQ(event1_set, true);
    EXPECT_NEAR(event1_time, 0.2, 1e-5);
    EXPECT_EQ(event2_set, true);
    EXPECT_NEAR(event2_time, 0.201, 1e-5);
}

TEST(testNLDiffSolver, testZeroCrossing)
{
    auto s1 = opt::SourceEq{ {0} ,[&](std::span<double> out, const double& t, opt::SourceEq&) {
            out[0] = t;
        }
    };
    double trigger_planned_time = 0.49;
    bool triggered = false;
    double triggered_time = 0;
    auto comparator = opt::NLStatefulEquation{ {0}, {1},
        opt::NLStatefulEquation::NLStatefulFunctor{[&](std::span<const double>, std::span<double> out, const double, const opt::NLStatefulEquation& eq)
        {
            bool triggered = eq.GetZeroCrossings()[0].current_value == opt::ZeroCrossDescriptor::Position::above;
            if (triggered)
            {
                out[0] = 1;
            }
            else
            {
                out[0] = 0;
            }
        }},
        opt::NLStatefulEquation::NLStatefulUpdateFunctor{[](std::span<const double> , const double, opt::NLStatefulEquation&)
        {

        }},
        opt::NLStatefulEquation::NLStatefulCrossFunctor{[&](const double& t, size_t, opt::NLStatefulEquation&) {
            triggered_time = t;
            triggered = true;
        }}
    };
    comparator.GetZeroCrossings().push_back(opt::ZeroCrossDescriptor{ trigger_planned_time, 0, opt::ZeroCrossDescriptor::CrossType::both, opt::ZeroCrossDescriptor::Position::undefined,
opt::ZeroCrossDescriptor::Position::undefined });
    opt::NLDiffSolver solver;
    solver.AddSource(std::move(s1));
    solver.AddNLStatefulEquation(std::move(comparator));

    opt::FlatMap state(2);
    state.modify(0, 1);
    state.modify(1, 0);

    double current_time = 0;

    solver.Initialize(0, 1);
    solver.SetMaxStep(0.01);
    solver.CalculateInitialConditions(state);
    while (solver.Step(state) != opt::StepResult::ReachedEnd)
    {
        EXPECT_GT(solver.GetCurrentTime(), current_time);
        current_time = solver.GetCurrentTime();
    }

    EXPECT_NEAR(1, solver.GetCurrentTime(), 1e-3);
    EXPECT_EQ(true, triggered);
    EXPECT_NEAR(trigger_planned_time, triggered_time, 1e-4);
}
