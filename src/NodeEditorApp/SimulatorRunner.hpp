#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <thread>
#include <variant>
#include <optional>
#include <vector>
#include "NodeModels/IdTypes.hpp"
#include <any>
#include "NodeEditorApp/SimulationManager.hpp"

namespace node
{

struct BlockResult
{
	model::BlockId id;
	std::any data;
};

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
	struct Stopped
	{

	};
	struct Success 
	{
		std::vector<BlockResult> result;
	};
	struct SimulationError
	{
		std::string error;
	};
	struct RequestedBadScene
	{
		SubSceneId subscene_id;
	};

	using Event_t = typename std::variant<Success, Stopped, NetFloatingError, OutputSocketsConflict, 
		FloatingInput, RequestedBadScene, SimulationError>;
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
	SimulatorRunner(std::vector<std::reference_wrapper<const model::NodeSceneModel>> models,
		SubSceneId main_subscene_id,
		std::shared_ptr<BlockClassesManager> classes_mgr, std::function<void()> end_callback, 
		SimulationSettings settings);
	~SimulatorRunner();

	std::optional<SimulationEvent>& GetResult() {
		return m_evt;
	}

	void Run();
	void Stop();
	bool IsEnded();
	bool IsStopped();
private:
	void RunImpl();
	SimulationEvent DoSimulation();

	std::function<void()> m_end_callback;
	std::atomic_bool m_ended{};
	std::atomic_bool m_stopped{};
	SubSceneId m_main_subscene_id;
	std::vector<std::unique_ptr<model::NodeSceneModel>> m_models;
	std::shared_ptr<BlockClassesManager> m_classes_mgr;
	std::optional<SimulationEvent> m_evt;
	SimulationSettings m_settings;
};

}