#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <thread>
#include <variant>
#include <optional>
#include <vector>
#include "NodeModels/IdTypes.hpp"

namespace node
{


struct SimulationEvent
{
	struct NetFloatingError
	{
		std::vector<model::NetNodeId> nodes;
	};
	struct OutputSocketsConflict
	{
		std::vector<model::NetNodeId> nodes;
		std::vector<model::SocketUniqueId> sockets;
	};
	struct FloatingInput
	{
		std::vector<model::SocketUniqueId> sockets;
	};

	struct Success {};

	using Event_t = typename std::variant<Success, NetFloatingError, OutputSocketsConflict, FloatingInput>;
	Event_t e;
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

	std::optional<std::reference_wrapper<SimulationEvent>> GetResult() { 
		if (m_evt)
		{
			return *m_evt;
		}
		return std::nullopt;
		}

	void Run();
	void Stop();
	bool IsEnded();
private:
	void RunImpl();
	SimulationEvent DoSimulation();

	std::function<void()> m_end_callback;
	std::atomic_flag m_ended;
	std::atomic_flag m_stopped;
	std::unique_ptr<model::NodeSceneModel> m_model;
	std::shared_ptr<BlockClassesManager> m_classes_mgr;
	std::thread m_thread;
	std::optional<SimulationEvent> m_evt;
};

}