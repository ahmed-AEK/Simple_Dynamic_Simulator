#include "gtest/gtest.h"
#include "optimizer/NLGraphSolver.hpp"
#include <numbers>
#include <cmath>

TEST(testNLGraphSolver, testConstructor)
{
	opt::NLEquation eq1{ { 1 }, { 2 }, [](auto in, auto out) {out[0] = in[0];} };
	opt::NLGraphSolver solver;
	solver.AddEquation(std::move(eq1));
}

TEST(testNLGraphSolver, testAddStateful)
{
	opt::NLGraphSolver solver;
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

TEST(testNLGraphSolver, testStateful_runs)
{
	opt::NLGraphSolver solver;
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

TEST(testNLGraphSolver, testEmptyRun)
{
	opt::FlatMap state;
	opt::NLGraphSolver solver;
	solver.Initialize();
	solver.Solve(state, 0);
}

TEST(testNLGraphSolver, testSolve)
{
	opt::NLEquation eq1{ { 0 }, { 1 }, [](auto in, auto out) {out[0] = in[0];} };
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
	opt::NLEquation eq1{ { 0 }, { 1 }, [](auto in, auto out) {out[0] = in[0];} };
	opt::NLEquation eq2{ { 1 } , { 2 }, [](auto in, auto out) {out[0] = in[0] * 2;} };
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
	opt::NLEquation eq1{ { 0 }, { 1 }, [](auto in, auto out) {out[0] = in[0] + 1;} }; // y = x + 1
	opt::NLEquation eq2{ { 1 } , { 0 }, [](auto in, auto out) {out[0] = in[0] * 2;} }; // x = 2 * y
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
	auto mul = opt::NLEquation{ {0,1}, {2}, [](std::span<const double> in, std::span<double> out) {out[0] = in[0] * in[1]; } };
	auto diff = opt::NLStatefulEquation{
		{2},
		{3},
		opt::NLStatefulEquation::NLStatefulFunctor{[](std::span<const double> in, std::span<double> out, const double t, const opt::FatAny& old_state) ->opt::FatAny
		{
			if (old_state.contains<std::array<double, 3>>())
			{
				auto& arr = old_state.get<std::array<double, 3>>();
				if (arr[0] == t)
				{
					out[0] = arr[2];
				}
				else
				{
					out[0] = (in[0] - arr[1]) / (t - arr[0]);
				}
			}
			else
			{
				out[0] = 0;
			}

			return opt::FatAny{ std::array<double, 3>{ t,in[0], out[0] }};
		}}
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