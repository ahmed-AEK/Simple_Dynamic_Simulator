
TEST(testNLEquation, testApply)
{
	
	opt::NLEquation eq({0, 1, 2 }, { 3 },
		opt::NLEquation::NLFunctor{[](auto inputs, auto output) -> void
		{
			output[0] = inputs[0] + inputs[1] + inputs[2];
		} } );
	
	std::vector<double> inputs{ 1,2,3 };
	std::vector<double> outputs{ 0 };
	auto input_buffer = eq.get_input_buffer();
	for (size_t i = 0 ; i < inputs.size(); i++)
	{
		input_buffer[i] = inputs[i];
	}
	eq.Apply();
	auto output_buffer = eq.get_output_buffer();

	EXPECT_EQ(output_buffer[0], 6);
	EXPECT_EQ(input_buffer.size(), 3);
	EXPECT_EQ(output_buffer.size(), 1);
}
