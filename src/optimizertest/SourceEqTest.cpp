#include "optimizer/SourceEq.hpp"
#include "gtest/gtest.h"
#include <memory_resource>
#include "toolgui/NodeMacros.h"

TEST(testSourceEq, testApply)
{
	std::array<double, 3> output_vals { 1, 2, 3 };
	opt::SourceEq eq({ 0, 1, 2, 3 },
		opt::SourceEq::SourceFunctor{ [&](auto outputs, const auto& t, opt::SourceEq&)
		{
			outputs[0] = output_vals[0];
			outputs[1] = output_vals[1];
			outputs[2] = output_vals[2];
			outputs[3] = t;
		} });

	eq.Apply(5);
	std::array<double, 4> outputs;
	auto output_buffer = eq.get_output_buffer();
	for (size_t i = 0; i < outputs.size(); i++)
	{
		outputs[i] = output_buffer[i];
	}
	std::array<double, 4> expected_outputs{ 1, 2, 3, 5 };

	EXPECT_EQ(output_buffer.size(), 4);
	for (size_t i = 0; i < outputs.size(); i++)
	{
		EXPECT_EQ(outputs[i], expected_outputs[i]) << "Vectors x and y differ at index " << i;
	}
}
