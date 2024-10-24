#pragma once

#include <memory>
#include <vector>
#include <functional>

namespace node
{
class SimulatorRunner;
struct BlockResult;
class GraphicsObjectsManager;
class BlockClassesManager;
struct SimulationEvent;
class Application;

namespace model
{
	class NodeSceneModel;
}

struct SimulationSettings
{
	double t_start = 0;
	double t_end = 10;
	double max_step = 0.01;
};

class SimulationManager
{
public:
	SimulationManager();
	SimulationManager(const SimulationManager&) = delete;
	SimulationManager& operator=(const SimulationManager&) = delete;
	~SimulationManager();

	void RunSimulator(model::NodeSceneModel& scene_model, std::shared_ptr<BlockClassesManager> classes_manager, Application& app);
	void StopSimulator();
	void CheckSimulatorEnded();
	bool IsSimulationRunning() const { return m_current_running_simulator != nullptr; }

	void SetSimulationSettings(SimulationSettings settings) { m_simulationSettings = settings; }
	const SimulationSettings& GetSimulationSettings() const { return m_simulationSettings; }

	const std::vector<BlockResult>& GetLastSimulationResults() const { return m_last_simulation_result; }
	void SetSimulationEndCallback(std::function<void(const SimulationEvent&)> end_callback);
	void ClearLastSimulationReults();
private:
	void OnSimulationEnd(SimulationEvent& evt);

	std::vector<BlockResult> m_last_simulation_result;
	std::shared_ptr<SimulatorRunner> m_current_running_simulator = nullptr;
	std::function<void(const SimulationEvent&)> m_end_callback;

	SimulationSettings m_simulationSettings;
};

}