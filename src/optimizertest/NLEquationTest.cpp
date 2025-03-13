
TEST(testNLEquation, testApply)
{
	
	opt::NLEquationWrapper eq({0, 1, 2 }, { 3 },
		opt::make_NLEqn<opt::FunctorNLEquation>([](auto inputs, auto output) -> void
		{
			output[0] = inputs[0] + inputs[1] + inputs[2];
		} ) );
	
	std::vector<double> inputs{ 1,2,3 };
	std::vector<double> outputs{ 0 };
	std::vector<double> input_buffer(inputs.size());
	std::vector<double> output_buffer(outputs.size());
	for (size_t i = 0 ; i < inputs.size(); i++)
	{
		input_buffer[i] = inputs[i];
	}
	eq.equation->Apply(input_buffer, output_buffer);
	
	EXPECT_EQ(output_buffer[0], 6);
	EXPECT_EQ(input_buffer.size(), 3);
	EXPECT_EQ(output_buffer.size(), 1);
}
