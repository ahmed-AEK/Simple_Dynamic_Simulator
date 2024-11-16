#include "gtest/gtest.h"
#include "optimizer/Equation.hpp"
#include <array>

TEST(testEquation, testIds)
{
	std::vector<int32_t> input_ids{ 0,1,2 };
	std::vector<int32_t> output_ids{ 3 };
	opt::InputEquation eq{ { input_ids.begin(), input_ids.end() } };
	opt::OutputEquation eq2{ { output_ids.begin(), output_ids.end() } };
	auto input_ids_buf = eq.get_input_ids();
	auto output_ids_buf = eq2.get_output_ids();

	EXPECT_EQ(input_ids_buf.size(), input_ids.size());
	EXPECT_EQ(output_ids_buf.size(), output_ids.size());
	for (size_t i = 0; i < input_ids.size(); i++)
	{
		EXPECT_EQ(input_ids_buf[i], input_ids[i]) << "Vectors x and y differ at index " << i;
	}
	EXPECT_EQ(output_ids_buf[0], output_ids[0]);
}


TEST(testEquation, testAllocator)
{
	std::array<char, 1024> buffer;
	std::pmr::monotonic_buffer_resource resource{ buffer.data(), buffer.size(), std::pmr::null_memory_resource()};
	opt::InputEquation eq({ { 0, 1, 2 }, &resource }, &resource);
	opt::OutputEquation eq2({ { 3 }, &resource }, &resource);

	std::vector<double> inputs{ 1,2,3 };
	auto input_buffer = eq.get_input_buffer();
	for (size_t i = 0; i < inputs.size(); i++)
	{
		input_buffer[i] = inputs[i];
	}
	auto output_buffer = eq2.get_output_buffer();

	EXPECT_EQ(input_buffer.size(), 3);
	EXPECT_EQ(output_buffer.size(), 1);
	EXPECT_GE((void*)input_buffer.data(), (void*)&buffer[0]);
	EXPECT_LE((void*)input_buffer.data(), (void*)&buffer[1023]);
	EXPECT_GE((void*)output_buffer.data(), (void*)&buffer[0]);
	EXPECT_LE((void*)output_buffer.data(), (void*)&buffer[1023]);

}