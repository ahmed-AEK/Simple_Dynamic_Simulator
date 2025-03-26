
TEST(testSourceEq, testApply)
{
	std::array<double, 3> output_vals { 1, 2, 3 };
	opt::SourceEqWrapper eq({ 0, 1, 2, 3 },
		opt::make_SourceEqn<opt::FunctorSourceEq>(opt::FunctorSourceEq::SourceFunctor{ [&](auto outputs, const auto& t, opt::SourceEvent&)
		{
			outputs[0] = output_vals[0];
			outputs[1] = output_vals[1];
			outputs[2] = output_vals[2];
			outputs[3] = t;
		} }));
	std::vector<double> output_buffer(eq.output_ids.size());

	opt::SourceEvent ev{};
	auto res = eq.equation->Apply(output_buffer, 5, ev);
	ASSERT_EQ(res, opt::Status::ok);

	std::array<double, 4> outputs;
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
