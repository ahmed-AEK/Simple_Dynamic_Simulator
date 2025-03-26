#include "optimizer/NLDiffSolver.hpp"
#include <iostream>
#include <chrono>
#include "toolgui/NodeMacros.h"

int main()
{
    int iterations = 0;
    const int count = 300000;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for (int i = 0; i < count; i++)
    {
        iterations = 0;
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