
TEST(testObserver, testApply)
{
	std::array<double, 4> outputs{ 0,0,0,0 };
	opt::Observer eq({ 0, 1, 2 },
		opt::Observer::ObserverFunctor{ [&](auto inputs, const auto& t)
		{
			outputs[0] = inputs[0];
			outputs[1] = inputs[1];
			outputs[2] = inputs[2];
			outputs[3] = t;
		} });

	std::vector<double> inputs{ 1,2,3 };
	auto input_buffer = eq.get_input_buffer();
	for (size_t i = 0; i < inputs.size(); i++)
	{
		input_buffer[i] = inputs[i];
	}
	eq.Apply(5);

	EXPECT_EQ(input_buffer.size(), 3);
	for (size_t i = 0; i < inputs.size(); i++)
	{
		EXPECT_EQ(inputs[i], outputs[i]) << "Vectors x and y differ at index " << i;
	}
}
