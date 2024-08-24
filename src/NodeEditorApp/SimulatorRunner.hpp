#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <thread>

namespace node
{

struct SimulationEvent
{

};

namespace model
{
	class NodeSceneModel;
}

class Application;
class MainNodeScene;
class BlockClassesManager;

class SimulatorRunner
{
public:
	SimulatorRunner(const model::NodeSceneModel& model, std::shared_ptr<BlockClassesManager> classes_mgr, std::function<void()> end_callback);
	~SimulatorRunner();
	SimulatorRunner(const SimulatorRunner&) = default;
	SimulatorRunner(SimulatorRunner&&) = default;
	SimulatorRunner& operator=(const SimulatorRunner&) = default;
	SimulatorRunner& operator=(SimulatorRunner&&) = default;

	void Run();
	void Stop();
	bool IsEnded();
private:
	void RunImpl();

	std::function<void()> m_end_callback;
	std::atomic_flag m_ended;
	std::atomic_flag m_stopped;
	std::unique_ptr<model::NodeSceneModel> m_model;
	std::shared_ptr<BlockClassesManager> m_classes_mgr;
	std::thread m_thread;
};

}