 

TEST(testDiffSolver, testConstructor)
{
    opt::DiffEquation eq({0},{1}, [](auto in, auto out, const auto& t) { return out[0] = in[0] + t;});
    opt::DiffSolver solver;
    solver.AddEquation(std::move(eq));
}

TEST(testDiffSolver, testInitialize)
{
    opt::DiffEquation eq({0},{1}, [](auto in, auto out, const auto& t) { return out[0] = in[0] + t;});
    opt::DiffSolver solver{};
    solver.AddEquation(std::move(eq));
    solver.Initialize(0, 10);

    EXPECT_EQ(solver.GetStartTime(), 0);
    EXPECT_EQ(solver.GetEndTime(), 10);
}

TEST(testDiffSolver, testStep_time_advances)
{
    opt::DiffEquation eq({ 0 }, { 1 }, [](auto in, auto out, const auto& t) { return out[0] = in[0]; UNUSED_PARAM(t);});
    opt::DiffSolver solver{};
    solver.AddEquation(std::move(eq));
    solver.Initialize(0, 2);
    opt::FlatMap state(2);
    state.modify(0,1);
    state.modify(1,0);
    double current_time = 0;

    while (solver.Step(state) != opt::StepResult::ReachedEnd)
    {
        EXPECT_GT(solver.GetCurrentTime(), current_time);
        current_time = solver.GetCurrentTime();
    }
    EXPECT_EQ(solver.GetCurrentTime(), 2);
    EXPECT_NEAR(state.get(1), 2, 2e-3);
}

TEST(testDiffSolver, testPreprocessor)
{
    opt::DiffEquation eq({ 0 }, { 1 }, [](auto in, auto out, const auto& t) { return out[0] = in[0]; UNUSED_PARAM(t);});
    opt::DiffSolver solver{};
    solver.AddEquation(std::move(eq));

    solver.Initialize(0, 2);
    opt::FlatMap state(2);
    state.modify(0,1);
    state.modify(1,0);

    bool called = false;
    double start_time = -1;
    double input_value = -1;
    auto preprocessor = [&](auto& state, const auto& time) -> void
        {
            if (called == false)
            {
                called = true;
                start_time = time;
                input_value = state.get(0);
            }
        };
    solver.SetPreprocessor(preprocessor);
    solver.Step(state);

    EXPECT_EQ(called, true);
    EXPECT_EQ(start_time, 0);
    EXPECT_EQ(input_value, 1);
}

TEST(testDiffSolver, test2Eq)
{
    opt::DiffEquation eq({ 0 }, { 1 }, [](auto in, auto out, const auto& t) { return out[0] = in[0]; UNUSED_PARAM(t);});
    opt::DiffEquation eq2({ 1 }, { 2 }, [](auto in, auto out, const auto& t) { return out[0] = in[0]; UNUSED_PARAM(t);});
    opt::DiffSolver solver;
    solver.AddEquation(eq);
    solver.AddEquation(eq2);
    solver.Initialize(0, 2);
    opt::FlatMap state(3);
    state.modify(0,1);
    state.modify(1,0);
    state.modify(2,0);
    double current_time = 0;

    while (solver.Step(state) != opt::StepResult::ReachedEnd)
    {
        EXPECT_GT(solver.GetCurrentTime(), current_time);
        current_time = solver.GetCurrentTime();
    }
    EXPECT_EQ(solver.GetCurrentTime(), 2);
    EXPECT_NEAR(state.get(1), 2, 2e-3);
    EXPECT_NEAR(state.get(2), 2, 2e-3);
}