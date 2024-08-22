#pragma once

#include <atomic>
#include <functional>

namespace node
{

struct SimulationEvent
{

};

class Application;
class MainNodeScene;

class SimulatorRunner
{
public:
	SimulatorRunner(std::function<void()> end_callback);
	void Run();
	void Stop();
	bool IsEnded();
private:
	std::function<void()> m_end_callback;
	std::atomic_flag m_ended;
};

}