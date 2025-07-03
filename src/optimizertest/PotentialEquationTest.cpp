
TEST(testPotentialEquation, testApply)
{

	opt::PotentialEquationWrapper eq({}, { 0, 1 }, 2,
		opt::make_PotentialEqn<opt::FunctorPotentialEquation>([](auto inputs, double flow, double& potential) -> void
			{
				UNUSED_PARAM(inputs);
				potential = flow;
			}));

	std::vector<double> inputs{};
	std::vector<double> input_buffer(inputs.size());
	for (size_t i = 0; i < inputs.size(); i++)
	{
		input_buffer[i] = inputs[i];
	}
	double out_value{};
	double flow = 1;
	auto res = eq.equation->Apply(input_buffer, flow, out_value);
	ASSERT_EQ(res, opt::Status::ok);

	EXPECT_EQ(out_value, flow);
}
