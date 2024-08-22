#include "SimulatorRunner.hpp"

node::SimulatorRunner::SimulatorRunner(std::function<void()> end_callback)
	:m_end_callback{std::move(end_callback)}
{
}

void node::SimulatorRunner::Run()
{
	m_ended.test_and_set();
	if (m_end_callback)
	{
		m_end_callback();
	}
}

void node::SimulatorRunner::Stop()
{
}

bool node::SimulatorRunner::IsEnded()
{
	return m_ended.test();
}
