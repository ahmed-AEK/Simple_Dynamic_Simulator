

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
    opt::DiffEquationWrapper eq({ 0 }, { 1 }, 
        opt::make_DiffEqn<opt::FunctorDiffEquation>([](auto in, auto out, auto t) { return out[0] = in[0]; UNUSED_PARAM(t);}));
    opt::NLDiffSolver solver;
    solver.AddDiffEquation(std::move(eq));
    solver.Initialize(0, 2);
    opt::FlatMap state(2);
    state.modify(0,1);
    state.modify(1,0);
    double current_time = 0;

    auto result1 = solver.CalculateInitialConditions(state);
    ASSERT_TRUE(result1);

    while (solver.Step(state).value() != opt::StepEnd::ReachedEnd)
    {
        EXPECT_GT(solver.GetCurrentTime(), current_time);
        current_time = solver.GetCurrentTime();
    }
    EXPECT_EQ(solver.GetCurrentTime(), 2);
    EXPECT_NEAR(state.get(1), 2, 2e-3);
}


TEST(testNLDiffSolver, testNLEquations_simple)
{
    opt::NLEquationWrapper eq({ 0 }, { 1 }, opt::make_NLEqn<opt::FunctorNLEquation>([](auto in, auto out) -> void { out[0] = in[0] * 2; }));
    opt::NLDiffSolver solver;
    solver.AddNLEquation(std::move(eq));
    solver.Initialize(0, 2);
    opt::FlatMap state(2);
    state.modify(0,1);
    state.modify(1,0);
    auto result = solver.CalculateInitialConditions(state);
    ASSERT_TRUE(result);

    EXPECT_NEAR(state.get(1), 2, 2e-3);
}

TEST(testNLDiffSolver, testStopOnErrorNL)
{
    static std::string Error_string = "This Error";
    struct ErrorTestNLFunctionNLDiff : public opt::INLEquation
    {
        opt::Status Apply(std::span<const double> input, std::span<double> output)
        {
            UNUSED_PARAM(input);
            UNUSED_PARAM(output);
            return opt::Status::error;
        }
        virtual void Destroy() { delete this; }
        virtual const char* GetLastError() { return Error_string.c_str(); }
        virtual void ClearError() {}
    };
    opt::NLEquationWrapper eq1{ { 0 }, { 1 }, opt::make_NLEqn<ErrorTestNLFunctionNLDiff>() };
    opt::NLDiffSolver solver;
    solver.AddNLEquation(std::move(eq1));

    solver.Initialize(0, 2);
    opt::FlatMap state(2);
    state.modify(0, 1);
    state.modify(1, 0);
    auto result = solver.CalculateInitialConditions(state);

    ASSERT_FALSE(result);
    EXPECT_EQ(Error_string, result.error());

}



TEST(testNLDiffSolver, testStopOnErrorDiff)
{
    static std::string Error_string = "This Error";
    struct ErrorTestDiffFunctionNLDiff : public opt::IDiffEquation
    {
        [[nodiscard]] opt::Status Apply(std::span<const double> input, std::span<double> output, const double t) override
        {
            UNUSED_PARAM(input);
            UNUSED_PARAM(output);
            UNUSED_PARAM(t);
            return opt::Status::error;
        }
        virtual void Destroy() { delete this; }
        virtual const char* GetLastError() { return Error_string.c_str(); }
        virtual void ClearError() {}
    };
    opt::DiffEquationWrapper eq1{ { 0 }, { 1 }, opt::make_DiffEqn<ErrorTestDiffFunctionNLDiff>() };
    opt::NLDiffSolver solver;
    solver.AddDiffEquation(std::move(eq1));

    solver.Initialize(0, 2);
    opt::FlatMap state(2);
    state.modify(0, 1);
    state.modify(1, 0);
    auto result = solver.CalculateInitialConditions(state);

    EXPECT_TRUE(result);

    auto result2 = solver.Step(state);
    ASSERT_FALSE(result2);
    EXPECT_EQ(Error_string, result2.error());

}
TEST(testNLDiffSolver, testNLEquations_simple_observer)
{
    opt::NLEquationWrapper eq({ 0 }, { 1 }, opt::make_NLEqn<opt::FunctorNLEquation>([](auto in, auto out) -> void { out[0] = in[0] * 2; }));
    opt::NLDiffSolver solver;

    std::vector<std::vector<double>> observer_log;
    opt::ObserverWrapper observer({ 0, 1 }, opt::make_ObserverEqn<opt::FunctorObserver>([&](auto inputs, const auto& t)
        {
            observer_log.push_back({ inputs[0], inputs[1], t });
        }));
    solver.AddObserver(std::move(observer));

    solver.AddNLEquation(std::move(eq));
    solver.Initialize(0, 2);
    opt::FlatMap state(2);
    state.modify(0, 1);
    state.modify(1, 0);
    auto result = solver.CalculateInitialConditions(state);
    ASSERT_TRUE(result);

    EXPECT_NEAR(state.get(1), 2, 2e-3);
    EXPECT_EQ(state.get(0), observer_log[0][0]);
    EXPECT_EQ(state.get(1), observer_log[0][1]);
    EXPECT_EQ(0, observer_log[0][2]);
}
TEST(testNLDiffSolver, testNLDiffEquations_1d1n)
{
    opt::DiffEquationWrapper eq({ 1 }, { 2 }, 
        opt::make_DiffEqn<opt::FunctorDiffEquation>([](auto in, auto out, auto t) -> void { out[0] = in[0]; UNUSED_PARAM(t);}));
    opt::NLEquationWrapper eq2({ 0 }, { 1 }, opt::make_NLEqn<opt::FunctorNLEquation>([](auto in, auto out) -> void { out[0] = in[0] * 2; }));
    opt::NLDiffSolver solver;
    solver.AddDiffEquation(std::move(eq));
    solver.AddNLEquation(std::move(eq2));
    solver.Initialize(0, 2);
    opt::FlatMap state(3);
    state.modify(0,1);
    state.modify(1,0);
    state.modify(2,0);
    double current_time = 0;

    auto result1 = solver.CalculateInitialConditions(state);
    ASSERT_TRUE(result1);

    while (solver.Step(state).value() != opt::StepEnd::ReachedEnd)
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
    opt::DiffEquationWrapper eq({ 1 }, { 2 }, 
        opt::make_DiffEqn<opt::FunctorDiffEquation>([](auto in, auto out, auto t) -> void { out[0] = in[0]; UNUSED_PARAM(t); }));
    opt::NLEquationWrapper eq2({ 0 }, { 1 }, opt::make_NLEqn<opt::FunctorNLEquation>([](auto in, auto out) -> void { out[0] = in[0] * 2; }));
    opt::NLDiffSolver solver;

    std::vector<std::vector<double>> observer_log;
    opt::ObserverWrapper observer({ 0, 1, 2 }, opt::make_ObserverEqn<opt::FunctorObserver>([&](auto inputs, const auto& t)
        {
            observer_log.push_back({ inputs[0], inputs[1], inputs[2], t});
        }));
    solver.AddObserver(std::move(observer));

    solver.AddDiffEquation(std::move(eq));
    solver.AddNLEquation(std::move(eq2));
    solver.Initialize(0, 2);
    opt::FlatMap state(3);
    state.modify(0, 1);
    state.modify(1, 0);
    state.modify(2, 0);
    double current_time = 0;

    auto result = solver.CalculateInitialConditions(state);
    ASSERT_TRUE(result);

    while (solver.Step(state).value() != opt::StepEnd::ReachedEnd)
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
    opt::DiffEquationWrapper eq({ 1 }, { 2 }, 
        opt::make_DiffEqn<opt::FunctorDiffEquation>([](auto in, auto out, auto t) -> void { out[0] = in[0]; UNUSED_PARAM(t);}));
    opt::NLEquationWrapper eq2({ 0 }, { 1 }, opt::make_NLEqn<opt::FunctorNLEquation>([](auto in, auto out) -> void { out[0] = in[0] * 2; }));
    opt::NLEquationWrapper eq3({ 2 }, { 3 }, opt::make_NLEqn<opt::FunctorNLEquation>([](auto in, auto out) -> void { out[0] = in[0] * 2; }));
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

    auto result1 = solver.CalculateInitialConditions(state);
    ASSERT_TRUE(result1);

    while (solver.Step(state).value() != opt::StepEnd::ReachedEnd)
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
    std::optional<DerivativeStateTest> deriv_state;
    auto s1 = opt::SourceEqWrapper{ {0} , opt::make_SourceEqn<opt::FunctorSourceEq>(
        [](std::span<double> out, const double& t, opt::SourceEvent&) { out[0] = std::sin(2 * std::numbers::pi * t); } ), {}};
    auto s2 = opt::SourceEqWrapper{ {1} , opt::make_SourceEqn<opt::FunctorSourceEq>(
        [](std::span<double> out, const double& t, opt::SourceEvent&) { out[0] = std::sin(2 * std::numbers::pi * t); } ), {}};
    auto mul = opt::NLEquationWrapper{ {0,1}, {2}, 
        opt::make_NLEqn<opt::FunctorNLEquation>([](std::span<const double> in, std::span<double> out) {out[0] = in[0] * in[1]; })};
    auto diff = opt::NLStatefulEquationWrapper{
        {2},
        {3},
        opt::make_NLStatefulEqn<opt::FunctorNLStatefulEquation>([state = &deriv_state](std::span<const double> in, 
            std::span<double> out, const double t, opt::NLStatefulEquationDataCRef)
        {
            const auto& old_state = *state;
            if (old_state)
            {
                const auto& state_val = *old_state;
                if (t == state_val.last_input_time)
                {
                    out[0] = state_val.last_out;
                }
                else
                {
                    out[0] = (in[0] - state_val.last_input) / (t - state_val.last_input_time);
                }
            }
            else
            {
                out[0] = 0;
            }
        },
        opt::FunctorNLStatefulEquation::NLStatefulUpdateFunctor{[state = &deriv_state](std::span<const double> in, 
            const double t, opt::NLStatefulEquationDataRef)
        {
            auto& old_state = *state;
            if (old_state)
            {
                auto& state_val = *old_state;
                state_val.last_out = (in[0] - state_val.last_input) / (t - state_val.last_input_time);;
                state_val.last_input = in[0];
                state_val.last_input_time = t;
            }
            else
            {
                old_state = DerivativeStateTest{in[0], t, 0};
            }
        }
    }
    ), {}};

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

    auto result1 = solver.CalculateInitialConditions(state);
    ASSERT_TRUE(result1);

    while (solver.Step(state).value() != opt::StepEnd::ReachedEnd)
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
    auto s1 = opt::SourceEqWrapper{ {0} , opt::make_SourceEqn<opt::FunctorSourceEq>([&](std::span<double> out, const double&, opt::SourceEvent&) {
            out[0] = 0;
        },
        [&](const double& t, opt::SourceEvent&)
        {
            event1_set = true;
            event1_time = t;
        }), {}
    };
    s1.ev = opt::SourceEvent{ true, false, 0.2 };

    bool event2_set = false;
    double event2_time = 0;
    auto s2 = opt::SourceEqWrapper{ {1} , opt::make_SourceEqn<opt::FunctorSourceEq>([&](std::span<double> out, const double&, opt::SourceEvent&) {
            out[0] = 0;
        },
        [&](const double& t, opt::SourceEvent&)
        {
            event2_set = true;
            event2_time = t;
        }
    ), {}
    };
    s2.ev = opt::SourceEvent{true, false, 0.201};

    opt::NLDiffSolver solver;
    solver.AddSource(std::move(s1));
    solver.AddSource(std::move(s2));

    opt::FlatMap state(2);
    double current_time = 0;

    solver.Initialize(0, 0.5);
    solver.SetMaxStep(0.01);
    auto result1 = solver.CalculateInitialConditions(state);
    ASSERT_TRUE(result1);

    while (solver.Step(state).value() != opt::StepEnd::ReachedEnd)
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
    auto s1 = opt::SourceEqWrapper{ {0} , opt::make_SourceEqn<opt::FunctorSourceEq>([&](std::span<double> out, const double& t, opt::SourceEvent&) {
            out[0] = t;
        }), {}
    };
    double trigger_planned_time = 0.49;
    bool triggered = false;
    double triggered_time = 0;
    auto comparator = opt::NLStatefulEquationWrapper{ {0}, {1},
        opt::make_NLStatefulEqn<opt::FunctorNLStatefulEquation>([&](std::span<const double>, 
            std::span<double> out, double, opt::NLStatefulEquationDataCRef data)
        {
            bool triggered = data.crossings[0].current_value == opt::ZeroCrossDescriptor::Position::above;
            if (triggered)
            {
                out[0] = 1;
            }
            else
            {
                out[0] = 0;
            }
        },
        opt::FunctorNLStatefulEquation::NLStatefulUpdateFunctor{[](std::span<const double> , double, opt::NLStatefulEquationDataRef)
        {

        }},
        opt::FunctorNLStatefulEquation::NLStatefulCrossFunctor{[&](double t, size_t, opt::NLStatefulEquationDataRef) {
            triggered_time = t;
            triggered = true;
        }}), {}
    };
    comparator.data.crossings.push_back(
        opt::ZeroCrossDescriptor{ trigger_planned_time, 0, 
        opt::ZeroCrossDescriptor::CrossType::both, opt::ZeroCrossDescriptor::Position::undefined,
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
    auto result1 = solver.CalculateInitialConditions(state);
    ASSERT_TRUE(result1);

    while (solver.Step(state).value() != opt::StepEnd::ReachedEnd)
    {
        EXPECT_GT(solver.GetCurrentTime(), current_time);
        current_time = solver.GetCurrentTime();
    }

    EXPECT_NEAR(1, solver.GetCurrentTime(), 1e-3);
    EXPECT_EQ(true, triggered);
    EXPECT_NEAR(trigger_planned_time, triggered_time, 1e-4);
}
