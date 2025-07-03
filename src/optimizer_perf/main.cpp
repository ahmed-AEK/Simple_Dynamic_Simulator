#include "optimizer/NLDiffSolver.hpp"
#include <iostream>
#include <chrono>
#include "toolgui/NodeMacros.h"

int main()
{
    int iterations = 0;
    const int count = 30000;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for (int i = 0; i < count; i++)
    {
        struct BufferFunctor : opt::IDiffEquation
        {
        public:
            virtual opt::Status Apply(std::span<const double> input, std::span<double> output, const double t) override
            {
                output[0] = input[0]; UNUSED_PARAM(t);
                return opt::Status::ok;
            }
            BufferFunctor() {}
        };
        struct MultiplyTwoFunctor : opt::INLEquation
        {
        public:
            opt::Status Apply(std::span<const double> input, std::span<double> output) override
            {
                output[0] = input[0] * 2;
                return opt::Status::ok;
            }
            MultiplyTwoFunctor() {}
        };

        iterations = 0;
        opt::DiffEquationWrapper eq({ 1 }, { 2 }, 
            opt::make_DiffEqn<BufferFunctor>());
        opt::NLEquationWrapper eq2({ 0 }, { 1 }, opt::make_NLEqn<MultiplyTwoFunctor>());
        opt::NLEquationWrapper eq3({ 2 }, { 3 }, opt::make_NLEqn<MultiplyTwoFunctor>());
        opt::NLDiffSolver solver;
        solver.AddDiffEquation(std::move(eq));
        solver.AddNLEquation(std::move(eq2));
        solver.AddNLEquation(std::move(eq3));
        [[maybe_unused]] auto _ = solver.Initialize(0, 20);
        opt::FlatMap state(4);
        state.modify(0,1);
        state.modify(1,0);
        state.modify(2,0);
        state.modify(3,0);
        
        auto result1 = solver.CalculateInitialConditions(state);
        if (!result1)
        {
            std::cout << "error: " << result1.error() << '\n';
            return -1;
        }
        while (solver.Step(state).value() != opt::StepEnd::ReachedEnd)
        {
            iterations++;
        }
    }

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / count << "[us]" << std::endl;
    std::cout << "iterations = " << iterations << '\n';
}