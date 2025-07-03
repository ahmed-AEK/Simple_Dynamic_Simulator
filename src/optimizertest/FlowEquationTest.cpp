
TEST(testFlowEquation, testApply)
{

	opt::FlowEquationWrapper eq({ 0, 1 }, { 0, 1 },
		opt::make_FlowEqn<opt::FunctorFlowEquation>([](auto inputs, auto output) -> void
			{
				output[0] = (inputs[0] - inputs[1]);
				output[1] = -(inputs[0] - inputs[1]);
			}));

	std::vector<double> inputs{ 1, 2 };
	std::vector<double> outputs{ -1, 1 };
	std::vector<double> input_buffer(inputs.size());
	std::vector<double> output_buffer(outputs.size());
	for (size_t i = 0; i < inputs.size(); i++)
	{
		input_buffer[i] = inputs[i];
	}
	auto res = eq.equation->Apply(input_buffer, output_buffer);
	ASSERT_EQ(res, opt::Status::ok);

	EXPECT_EQ(output_buffer[0], outputs[0]);
	EXPECT_EQ(output_buffer[1], outputs[1]);
}
