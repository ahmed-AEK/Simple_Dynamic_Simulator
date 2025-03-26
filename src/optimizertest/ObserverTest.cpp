
TEST(testObserver, testApply)
{
	std::array<double, 4> outputs{ 0,0,0,0 };
	opt::ObserverWrapper eq({ 0, 1, 2 },
		opt::make_ObserverEqn<opt::FunctorObserver>(opt::FunctorObserver::ObserverFunctor{ [&](auto inputs, const auto& t)
		{
			outputs[0] = inputs[0];
			outputs[1] = inputs[1];
			outputs[2] = inputs[2];
			outputs[3] = t;
		} }));

	std::vector<double> inputs{ 1,2,3 };
	auto input_buffer = std::vector<double>(inputs.size());
	for (size_t i = 0; i < inputs.size(); i++)
	{
		input_buffer[i] = inputs[i];
	}
	auto res = eq.equation->Apply(input_buffer, 5);
	ASSERT_EQ(res, opt::Status::ok);

	EXPECT_EQ(input_buffer.size(), 3);
	for (size_t i = 0; i < inputs.size(); i++)
	{
		EXPECT_EQ(inputs[i], outputs[i]) << "Vectors x and y differ at index " << i;
	}
}
