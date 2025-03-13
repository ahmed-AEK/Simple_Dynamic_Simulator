

TEST(testNLStatefulEquation, testApply)
{

	double state;
	opt::NLStatefulEquationWrapper eq({ 0, 1, 2 }, { 3 },
		opt::make_NLStatefulEqn<opt::FunctorNLStatefulEquation>([](auto inputs, auto output, const auto&, const auto&)
		{
			output[0] = inputs[0] + inputs[1] + inputs[2];
		},
		opt::FunctorNLStatefulEquation::NLStatefulUpdateFunctor{ [state_ptr = &state](std::span<const double>, double, opt::NLStatefulEquationDataRef)
		{
			*state_ptr = 5;
		}
		})
		);

	std::vector<double> inputs{ 1,2,3 };
	std::vector<double> outputs{ 0 };
	auto input_buffer = std::vector<double>(inputs.size());
	auto output_buffer = std::vector<double>(outputs.size());
	for (size_t i = 0; i < inputs.size(); i++)
	{
		input_buffer[i] = inputs[i];
	}
	eq.equation->Update(input_buffer, 0, eq.data);
	eq.equation->Apply(input_buffer, output_buffer, 0, eq.data);
	auto result = state;

	EXPECT_EQ(result, 5.0);
	EXPECT_EQ(output_buffer[0], 6);
	EXPECT_EQ(input_buffer.size(), 3);
	EXPECT_EQ(output_buffer.size(), 1);
}
