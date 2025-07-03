


TEST(testNLGraphSolver_flow, testAddFlowEqn)
{
	opt::NLGraphSolver solver;
	solver.AddFlowEquation(opt::FlowEquationWrapper{ { 0, 1 }, { 0, 1 },
		opt::make_FlowEqn<opt::FunctorFlowEquation>([](auto inputs, auto output) -> void
			{
				output[0] = (inputs[0] - inputs[1]);
				output[1] = -(inputs[0] - inputs[1]);
			}) });
	solver.AddFlowEquation(opt::FlowEquationWrapper{ { 1, 2 }, { 1, 2 },
	opt::make_FlowEqn<opt::FunctorFlowEquation>([](auto inputs, auto output) -> void
		{
			output[0] = (inputs[0] - inputs[1]);
			output[1] = -(inputs[0] - inputs[1]);
		}) });
	std::vector<int32_t> fixed_ids{ 0,2 };
	auto initialized = solver.Initialize(fixed_ids);
	ASSERT_TRUE(initialized);

	opt::FlatMap state(3);
	state.modify(0, 1);
	state.modify(1, 0);
	state.modify(2, 0);

	auto result1 = solver.Solve(state, 1);
	ASSERT_TRUE(result1);
	
	EXPECT_EQ(state.get(0), 1.0);
	EXPECT_NEAR(state.get(1), 0.5, 1e-3);
	EXPECT_EQ(state.get(2), 0.0);
}

TEST(testNLGraphSolver_flow, testFloatingFlowEqn)
{
	opt::NLGraphSolver solver;
	solver.AddFlowEquation(opt::FlowEquationWrapper{ { 0, 1 }, { 0, 1 },
		opt::make_FlowEqn<opt::FunctorFlowEquation>([](auto inputs, auto output) -> void
			{
				output[0] = (inputs[0] - inputs[1]);
				output[1] = -(inputs[0] - inputs[1]);
			}) });
	std::vector<int32_t> fixed_ids{ 0 };
	auto initialized = solver.Initialize(fixed_ids);
	ASSERT_TRUE(initialized);

	opt::FlatMap state(2);
	state.modify(0, 1);
	state.modify(1, 0);


	auto result1 = solver.Solve(state, 1);
	ASSERT_TRUE(result1);

	EXPECT_EQ(state.get(0), 1.0);
	EXPECT_NEAR(state.get(1), 1, 1e-3);
}


TEST(testNLGraphSolver_flow, testFloatingWithBlockFlowEqn)
{
	opt::NLGraphSolver solver;
	solver.AddFlowEquation(opt::FlowEquationWrapper{ { 0, 1 }, { 0, 1 },
		opt::make_FlowEqn<opt::FunctorFlowEquation>([](auto inputs, auto output) -> void
			{
				output[0] = (inputs[0] - inputs[1]);
				output[1] = -(inputs[0] - inputs[1]);
			}) });
	solver.AddEquation(opt::NLEquationWrapper{ {}, { 0 }, 
		opt::make_NLEqn<opt::FunctorNLEquation>([](auto in, auto out) {out[0] = 2; UNUSED_PARAM(in); }) }
	);
	std::vector<int32_t> fixed_ids{};
	auto initialized = solver.Initialize(fixed_ids);
	ASSERT_TRUE(initialized);

	opt::FlatMap state(2);
	state.modify(0, 0);
	state.modify(1, 0);


	auto result1 = solver.Solve(state, 1);
	ASSERT_TRUE(result1);

	EXPECT_EQ(state.get(0), 2.0);
	EXPECT_NEAR(state.get(1), 2, 1e-3);
}

TEST(testNLGraphSolver_flow, testFlowFixedTwoSides)
{
	opt::NLGraphSolver solver;
	solver.AddFlowEquation(opt::FlowEquationWrapper{ { 0, 1 }, { 0, 1 },
		opt::make_FlowEqn<opt::FunctorFlowEquation>([](auto inputs, auto output) -> void
			{
				output[0] = (inputs[0] - inputs[1]);
				output[1] = -(inputs[0] - inputs[1]);
			}) });
	std::vector<int32_t> fixed_ids{ 0, 1 };
	auto initialized = solver.Initialize(fixed_ids);
	ASSERT_TRUE(initialized);

	opt::FlatMap state(2);
	state.modify(0, 1);
	state.modify(1, 0);


	auto result1 = solver.Solve(state, 1);
	ASSERT_TRUE(result1);

	EXPECT_EQ(state.get(0), 1.0);
	EXPECT_EQ(state.get(1), 0);
}

TEST(testNLGraphSolver_flow, testFlowBlocksAtTwoSides)
{
	opt::NLGraphSolver solver;
	solver.AddFlowEquation(opt::FlowEquationWrapper{ { 0, 1 }, { 0, 1 },
		opt::make_FlowEqn<opt::FunctorFlowEquation>([](auto inputs, auto output) -> void
			{
				output[0] = (inputs[0] - inputs[1]);
				output[1] = -(inputs[0] - inputs[1]);
			}) });
	solver.AddEquation(opt::NLEquationWrapper{ {}, { 0, 1 },
		opt::make_NLEqn<opt::FunctorNLEquation>([](auto in, auto out) {out[0] = 2; out[1] = 3; UNUSED_PARAM(in); }) }
	);
	std::vector<int32_t> fixed_ids{};
	auto initialized = solver.Initialize(fixed_ids);
	ASSERT_TRUE(initialized);

	opt::FlatMap state(2);
	state.modify(0, 0);
	state.modify(1, 0);

	auto result1 = solver.Solve(state, 1);
	ASSERT_TRUE(result1);

	EXPECT_EQ(state.get(0), 2);
	EXPECT_EQ(state.get(1), 3);
}

TEST(testNLGraphSolver_flow, testAddPotentialEqn)
{
	opt::NLGraphSolver solver;
	solver.AddPotentialEquation(opt::PotentialEquationWrapper{ {}, { 0, 1 }, 2,
		opt::make_PotentialEqn<opt::FunctorPotentialEquation>([](auto inputs, double flow, double& potential) -> void
			{
				UNUSED_PARAM(inputs);
				UNUSED_PARAM(flow);
				potential = 1;
			}) });
	std::vector<int32_t> fixed_ids{ 0 };
	auto initialized = solver.Initialize(fixed_ids);
	ASSERT_TRUE(initialized);

	opt::FlatMap state(3);
	state.modify(0, 1);
	state.modify(1, 0);
	state.modify(2, 0);

	auto result1 = solver.Solve(state, 1);
	ASSERT_TRUE(result1);

	EXPECT_EQ(state.get(0), 1);
	EXPECT_NEAR(state.get(1), 2, 1e-3);
	EXPECT_NEAR(state.get(2), 0, 1e-3);
}

TEST(testNLGraphSolver_flow, testPotentialFloatingWithBlock)
{
	opt::NLGraphSolver solver;
	solver.AddPotentialEquation(opt::PotentialEquationWrapper{ {}, { 0, 1 }, 2,
		opt::make_PotentialEqn<opt::FunctorPotentialEquation>([](auto inputs, double flow, double& potential) -> void
			{
				UNUSED_PARAM(inputs);
				UNUSED_PARAM(flow);
				potential = 1;
			}) });
	solver.AddEquation(opt::NLEquationWrapper{ {}, { 0 },
		opt::make_NLEqn<opt::FunctorNLEquation>([](auto in, auto out) {out[0] = 2; UNUSED_PARAM(in); }) }
	);
	std::vector<int32_t> fixed_ids{};
	auto initialized = solver.Initialize(fixed_ids);
	ASSERT_TRUE(initialized);

	opt::FlatMap state(3);
	state.modify(0, 0);
	state.modify(1, 0);

	auto result1 = solver.Solve(state, 1);
	ASSERT_TRUE(result1);

	EXPECT_EQ(state.get(0), 2);
	EXPECT_NEAR(state.get(1), 3, 1e-3);
	EXPECT_NEAR(state.get(2), 0, 1e-3);
}


TEST(testNLGraphSolver_flow, testPotentialAndFlowParallel)
{
	opt::NLGraphSolver solver;
	solver.AddPotentialEquation(opt::PotentialEquationWrapper{ {}, { 0, 1 }, 2,
		opt::make_PotentialEqn<opt::FunctorPotentialEquation>([](auto inputs, double flow, double& potential) -> void
			{
				UNUSED_PARAM(inputs);
				UNUSED_PARAM(flow);
				potential = 1;
			}) });
	solver.AddFlowEquation(opt::FlowEquationWrapper{ { 0, 1 }, { 0, 1 },
	opt::make_FlowEqn<opt::FunctorFlowEquation>([](auto inputs, auto output) -> void
		{
			const double resistance = 10;
			output[0] = (inputs[0] - inputs[1]) / resistance;
			output[1] = -(inputs[0] - inputs[1]) / resistance;
		}) });
	solver.AddEquation(opt::NLEquationWrapper{ {}, { 0 },
		opt::make_NLEqn<opt::FunctorNLEquation>([](auto in, auto out) {out[0] = 1; UNUSED_PARAM(in); }) }
	);
	std::vector<int32_t> fixed_ids{};
	auto initialized = solver.Initialize(fixed_ids);
	ASSERT_TRUE(initialized);

	opt::FlatMap state(3);
	state.modify(0, 0);
	state.modify(1, 0);

	auto result1 = solver.Solve(state, 1);
	ASSERT_TRUE(result1);

	EXPECT_EQ(state.get(0), 1);
	EXPECT_NEAR(state.get(1), 2, 1e-3);
	EXPECT_NEAR(state.get(2), 0.1, 1e-3);
}


TEST(testNLGraphSolver_flow, testPotentialTwoSidesFixed)
{
	opt::NLGraphSolver solver;
	solver.AddPotentialEquation(opt::PotentialEquationWrapper{ {}, { 0, 1 }, 2,
		opt::make_PotentialEqn<opt::FunctorPotentialEquation>([](auto inputs, double flow, double& potential) -> void
			{
				UNUSED_PARAM(inputs);
				UNUSED_PARAM(flow);
				potential = 1;
			}) });
	std::vector<int32_t> fixed_ids{ 0, 1 };
	auto result = solver.Initialize(fixed_ids);

	ASSERT_FALSE(result.has_value());
}


TEST(testNLGraphSolver_flow, testPotentialFloating)
{
	opt::NLGraphSolver solver;
	solver.AddPotentialEquation(opt::PotentialEquationWrapper{ {}, { 0, 1 }, 2,
		opt::make_PotentialEqn<opt::FunctorPotentialEquation>([](auto inputs, double flow, double& potential) -> void
			{
				UNUSED_PARAM(inputs);
				UNUSED_PARAM(flow);
				potential = 1;
			}) });
	std::vector<int32_t> fixed_ids{};
	auto result = solver.Initialize(fixed_ids);

	ASSERT_FALSE(result.has_value());
}