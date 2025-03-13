

TEST(testNLGraphSolver, testConstructor)
{
	opt::NLEquationWrapper eq1{ { 1 }, { 2 }, opt::make_NLEqn<opt::FunctorNLEquation>([](auto in, auto out) {out[0] = in[0];})};
	opt::NLGraphSolver solver;
	solver.AddEquation(std::move(eq1));
}

TEST(testNLGraphSolver, testAddStateful)
{
	opt::NLGraphSolver solver;
	solver.AddStatefulEquation({ {}, { 0 }, opt::make_NLStatefulEqn<opt::FunctorNLStatefulEquation>([](auto, auto out, const auto& t, const auto&)
		{
			out[0] = t;
			return opt::FatAny{ t };
		},
		[](auto, auto, auto)
		{
		})
		});
	solver.Initialize();
	opt::FlatMap state(1);
	state.modify(0,0);
	
	solver.Solve(state, 1);
	solver.UpdateState(state, 1);

	EXPECT_NEAR(state.get(0), 1, 1e-4);
}

TEST(testNLGraphSolver, testAddBuffer)
{
	opt::NLGraphSolver solver;
	solver.AddBufferEquation({0, 1});
	solver.Initialize();
	opt::FlatMap state(2);
	state.modify(0, 5);
	state.modify(1, 0);
	
	solver.Solve(state, 1);
	solver.UpdateState(state, 1);

	auto first_output = state.get(1);
	state.modify(0, 7);

	solver.Solve(state, 1);
	solver.UpdateState(state, 1);

	auto second_output = state.get(1);

	EXPECT_EQ(first_output, 5);
	EXPECT_EQ(second_output, 7);
}

TEST(testNLGraphSolver, testStateful_runs)
{
	opt::NLGraphSolver solver;
	double recorded_double = 0;
	std::optional<double> stateful_state;
	solver.AddStatefulEquation({ {}, { 0 }, 
		opt::make_NLStatefulEqn<opt::FunctorNLStatefulEquation>([state = &stateful_state, recorded_double = &recorded_double](auto, auto out, const auto& t, opt::NLStatefulEquationDataCRef)
		{
			if (*state)
			{
				*recorded_double = state->value();
			}
			out[0] = t;
		},
		[state = &stateful_state](auto, const auto& t, opt::NLStatefulEquationDataRef)
		{
			*state = t;
		})
		});
	solver.Initialize();
	opt::FlatMap state(1);
	state.modify(0,0);

	solver.Solve(state, 1);
	solver.UpdateState(state, 1);

	auto first_state = state.get(0);
	solver.Solve(state, 2);
	solver.UpdateState(state, 2);
	auto second_state = state.get(0);

	EXPECT_NEAR(first_state, 1, 1e-4);
	EXPECT_EQ(recorded_double, 1);
	EXPECT_NEAR(second_state, 2, 1e-4);
}

TEST(testNLGraphSolver, testEmptyRun)
{
	opt::FlatMap state;
	opt::NLGraphSolver solver;
	solver.Initialize();
	solver.Solve(state, 0);
}

TEST(testNLGraphSolver, testSolve)
{
	opt::NLEquationWrapper eq1{ { 0 }, { 1 }, opt::make_NLEqn<opt::FunctorNLEquation>([](auto in, auto out) {out[0] = in[0];})};
	opt::NLGraphSolver solver;
	solver.AddEquation(std::move(eq1));

	solver.Initialize();
	opt::FlatMap state(2);
	state.modify(0,3);
	state.modify(1,0);
	solver.Solve(state, 0);
	solver.UpdateState(state, 0);
	EXPECT_NEAR(3.0, state.get(1), 1e-4);
}


TEST(testNLGraphSolver, testSolve_two_equations)
{
	opt::NLEquationWrapper eq1{ { 0 }, { 1 },  opt::make_NLEqn<opt::FunctorNLEquation>([](auto in, auto out) {out[0] = in[0];} )};
	opt::NLEquationWrapper eq2{ { 1 } , { 2 }, opt::make_NLEqn<opt::FunctorNLEquation>([](auto in, auto out) {out[0] = in[0] * 2;})};
	opt::NLGraphSolver solver;
	solver.AddEquation(std::move(eq1));
	solver.AddEquation(std::move(eq2));
	solver.Initialize();
	opt::FlatMap state(3);
	state.modify(0,3);
	state.modify(1,0);
	state.modify(2,0);
	solver.Solve(state, 0);
	solver.UpdateState(state, 0);
	EXPECT_NEAR(3.0, state.get(1), 1e-3);
	EXPECT_NEAR(6.0, state.get(2), 1e-3);
}

TEST(testNLGraphSolver, testSolve_two_equations_cyclic)
{
	opt::NLEquationWrapper eq1{ { 0 }, { 1 },  opt::make_NLEqn<opt::FunctorNLEquation>([](auto in, auto out) {out[0] = in[0] + 1;})}; // y = x + 1
	opt::NLEquationWrapper eq2{ { 1 } , { 0 }, opt::make_NLEqn<opt::FunctorNLEquation>([](auto in, auto out) {out[0] = in[0] * 2;})}; // x = 2 * y
	opt::NLGraphSolver solver;

	solver.AddEquation(std::move(eq1));
	solver.AddEquation(std::move(eq2));
	solver.Initialize();
	opt::FlatMap state(2);
	state.modify(0,0);
	state.modify(1,0);
	solver.Solve(state, 0);
	solver.UpdateState(state, 0);
	EXPECT_NEAR(-1.0, state.get(1), 1e-4); // y = -1
	EXPECT_NEAR(-2.0, state.get(0), 2e-4); // x = -2
}


TEST(testNLGraphSolver, testSolve_multiply_diff)
{
	struct DerivativeStateTest
	{
		double last_input;
		double last_input_time;
		double last_out;
	};

	std::optional<DerivativeStateTest> stateful_state;
	auto mul = opt::NLEquationWrapper{ {0,1}, {2}, 
		opt::make_NLEqn<opt::FunctorNLEquation>([](std::span<const double> in, std::span<double> out) {out[0] = in[0] * in[1]; })};
	auto diff = opt::NLStatefulEquationWrapper{
		{2},
		{3},
		opt::make_NLStatefulEqn<opt::FunctorNLStatefulEquation>(
			[state = &stateful_state](std::span<const double> in, std::span<double> out, double t, opt::NLStatefulEquationDataCRef)
		{
			if (*state)
			{
				auto& state_val = state->value();
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
		opt::FunctorNLStatefulEquation::NLStatefulUpdateFunctor{
				[state = &stateful_state](std::span<const double> in, double t, opt::NLStatefulEquationDataRef)
		{
			if (*state)
			{
				auto& state_val = state->value();
				state_val.last_out = (in[0] - state_val.last_input) / (t - state_val.last_input_time);;
				state_val.last_input = in[0];
				state_val.last_input_time = t;
			}
			else
			{
				*state = DerivativeStateTest{in[0], t, 0};
			}
		}
		})
	};

	opt::NLGraphSolver solver;
	solver.AddEquation(std::move(mul));
	solver.AddStatefulEquation(std::move(diff));

	solver.Initialize();
	opt::FlatMap state(4);
	state.modify(0, 0);
	state.modify(1, 0);
	state.modify(2, 0);
	state.modify(3, 0);
	solver.Solve(state, 0);
	solver.UpdateState(state, 0);
	
	state.modify(0, 0.01);
	state.modify(1, 0.01);
	solver.Solve(state, 1);
	solver.UpdateState(state, 0.003);

	auto expected_multiply_result = 0.0001;

	EXPECT_NEAR(expected_multiply_result, state.get(2), 1e-4);
}