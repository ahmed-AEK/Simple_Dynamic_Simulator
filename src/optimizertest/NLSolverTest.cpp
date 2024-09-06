#include "gtest/gtest.h"
#include "optimizer/NLSolver.hpp"

TEST(testNLSolver, testConstructor)
{
	opt::NLEquation eq1{ { 1 }, { 2 }, [](auto in, auto out) {out[0] = in[0];} };
	opt::NLSolver solver({std::move(eq1)});
}

TEST(testNLSolver, testAddStateful)
{
	opt::NLSolver solver;
	solver.AddStatefulEquation({ {}, { 0 }, [](auto, auto out, const auto& t, const auto&) ->opt::FatAny
		{
			out[0] = t;
			return opt::FatAny{ t };
		} });
	solver.Initialize();
	opt::FlatMap state(1);
	state.modify(0,0);
	
	solver.Solve(state, 1);
	solver.UpdateState(state, 1);

	EXPECT_NEAR(state.get(0), 1, 1e-4);
}

TEST(testNLSolver, testStateful_runs)
{
	opt::NLSolver solver;
	double recorded_double = 0;
	solver.AddStatefulEquation({ {}, { 0 }, [&](auto, auto out, const auto& t, const opt::FatAny& state) ->opt::FatAny
		{
			if (state.contains<double>())
			{
				recorded_double = state.get<double>();
			}
			out[0] = t;
			return opt::FatAny{ t };
		} });
	solver.Initialize();
	opt::FlatMap state(1);
	state.modify(0,0);

	solver.Solve(state, 1);
	solver.UpdateState(state, 1);

	solver.Solve(state, 2);
	solver.UpdateState(state, 2);

	EXPECT_EQ(recorded_double, 1);
	EXPECT_NEAR(state.get(0), 2, 1e-4);
}

TEST(testNLSolver, testEmptyRun)
{
	opt::FlatMap state;
	opt::NLSolver solver;
	solver.Initialize();
	solver.Solve(state, 0);
}

TEST(testNLSolver, testSolve)
{
	opt::NLEquation eq1{ { 0 }, { 1 }, [](auto in, auto out) {out[0] = in[0];} };
	opt::NLSolver solver({std::move(eq1)});
	solver.Initialize();
	opt::FlatMap state(2);
	state.modify(0,3);
	state.modify(1,0);
	solver.Solve(state, 0);
	solver.UpdateState(state, 0);
	EXPECT_NEAR(3.0, state.get(1), 1e-4);
}


TEST(testNLSolver, testSolve_two_equations)
{
	opt::NLEquation eq1{ { 0 }, { 1 }, [](auto in, auto out) {out[0] = in[0];} };
	opt::NLEquation eq2{ { 1 } , { 2 }, [](auto in, auto out) {out[0] = in[0] * 2;} };
	opt::NLSolver solver;
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

TEST(testNLSolver, testSolve_two_equations_cyclic)
{
	opt::NLEquation eq1{ { 0 }, { 1 }, [](auto in, auto out) {out[0] = in[0] + 1;} }; // y = x + 1
	opt::NLEquation eq2{ { 1 } , { 0 }, [](auto in, auto out) {out[0] = in[0] * 2;} }; // x = 2 * y
	opt::NLSolver solver({std::move(eq1), std::move(eq2)});
	solver.Initialize();
	opt::FlatMap state(2);
	state.modify(0,0);
	state.modify(1,0);
	solver.Solve(state, 0);
	solver.UpdateState(state, 0);
	EXPECT_NEAR(-1.0, state.get(1), 1e-4); // y = -1
	EXPECT_NEAR(-2.0, state.get(0), 2e-4); // x = -2
}