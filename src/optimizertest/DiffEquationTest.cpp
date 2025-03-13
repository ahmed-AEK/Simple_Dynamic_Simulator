
TEST(testDiffEquation, testApply)
{

	opt::DiffEquationWrapper eq{ {0, 1, 2 }, { 3 },
		opt::make_DiffEqn<opt::FunctorDiffEquation>([](auto inputs, auto output, const auto t) -> void
		{
			output[0] = inputs[0] + inputs[1] + inputs[2] + t;
		}) };
	
	std::vector<double> inputs{ 1,2,3 };
	std::vector<double> outputs{ 0 };
	std::vector<double> input_buffer(inputs.size());
	std::vector<double> output_buffer(outputs.size());
	for (size_t i = 0 ; i < inputs.size(); i++)
	{
		input_buffer[i] = inputs[i];
	}
	eq.equation->Apply(input_buffer, output_buffer, 1);

	EXPECT_EQ(output_buffer[0], 7);
	EXPECT_EQ(input_buffer.size(), 3);
	EXPECT_EQ(output_buffer.size(), 1);
}