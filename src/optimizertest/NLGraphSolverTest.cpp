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
	solver.AddStatefulEquation({ {}, { 0 }, [](auto, auto out, const auto& t, const auto&)
		{
			out[0] = t;
			return opt::FatAny{ t };
		},
		[](auto, const auto& , auto&)
		{
		}
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
	solver.AddStatefulEquation({ {}, { 0 }, [&](auto, auto out, const auto& t, const opt::NLStatefulEquation& eq)
		{
			const auto& state = eq.GetState();
			if (state.contains<double>())
			{
				recorded_double = state.get<double>();
			}
			out[0] = t;
		},
		[&](auto, const auto& t, opt::NLStatefulEquation& eq)
		{
			auto& state = eq.GetState();
			state = opt::FatAny{ t };
		}
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
	struct DerivativeStateTest
	{
		double last_input;
		double last_input_time;
		double last_out;
	};

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