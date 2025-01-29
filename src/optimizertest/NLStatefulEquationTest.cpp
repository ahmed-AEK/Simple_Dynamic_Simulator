

TEST(testNLStatefulEquation, testApply)
{

	opt::NLStatefulEquation eq({ 0, 1, 2 }, { 3 },
		opt::NLStatefulEquation::NLStatefulFunctor{ [](auto inputs, auto output, const auto&, const auto&)
		{
			output[0] = inputs[0] + inputs[1] + inputs[2];
		} },
		opt::NLStatefulEquation::NLStatefulUpdateFunctor{ [](auto , const auto&, opt::NLStatefulEquation& eq)
		{
			eq.GetState() = opt::FatAny{double{5}};
		}
		}
		);

	std::vector<double> inputs{ 1,2,3 };
	std::vector<double> outputs{ 0 };
	auto input_buffer = eq.get_input_buffer();
	for (size_t i = 0; i < inputs.size(); i++)
	{
		input_buffer[i] = inputs[i];
	}
	eq.Update(0);
	eq.Apply(0);
	auto&& result = eq.GetState();
	auto output_buffer = eq.get_output_buffer();

	EXPECT_EQ(result.contains<double>(), true);
	EXPECT_EQ(result.get<double>(), 5.0);
	EXPECT_EQ(output_buffer[0], 6);
	EXPECT_EQ(input_buffer.size(), 3);
	EXPECT_EQ(output_buffer.size(), 1);
}
