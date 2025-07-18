#include "SimulationManager.hpp"
#include "NodeEditorApp/SimulatorRunner.hpp"
#include "toolgui/Application.hpp"

#include <cassert>

// helper type for the visitor #4
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;


node::SimulationManager::SimulationManager()
{
}

node::SimulationManager::~SimulationManager()
{
    StopSimulator();
}

void node::SimulationManager::RunSimulator(SceneManagerId scene_session_id, 
    std::vector<std::reference_wrapper<const model::NodeSceneModel>> scene_models, 
    SubSceneId main_subscene_id,
    std::shared_ptr<BlockClassesManager> classes_manager, Application& app)
{
    assert(!m_current_running_simulator);
    if (!m_current_running_simulator)
    {
        auto runner = std::make_shared<SimulatorRunner>(
            scene_models,
            main_subscene_id,
            std::move(classes_manager),
            std::function<void()>{},
            m_simulationSettings);
        m_current_running_simulator = runner;
        m_last_simulation_scene_session_id = scene_session_id;
        app.DispatchTask([](std::shared_ptr<SimulatorRunner> runner, SimulationManager* manager, Application& app) -> Task
            {
                co_await app.DispatchThreadedTask([runner]() { runner->Run(); return 0; });
                manager->OnSimulationEnd(*runner->GetResult());
            }(runner, this, app));
    }
}

void node::SimulationManager::StopSimulator()
{
    if (m_current_running_simulator)
    {
        m_current_running_simulator->Stop();
    }
}

void node::SimulationManager::SetSimulationEndCallback(std::function<void(const SimulationEvent&)> end_callback)
{
    m_end_callback = std::move(end_callback);
}

void node::SimulationManager::ClearLastSimulationReults()
{
    m_last_simulation_result.clear();
}

void node::SimulationManager::OnSimulationEnd(SimulationEvent& evt)
{
    if (m_current_running_simulator && m_current_running_simulator->IsStopped())
    {
        logger(logging::LogCategory::Simulator).LogDebug("Stopped!");
        m_current_running_simulator = nullptr;
        return;
    }
    std::visit(overloaded{
        [](SimulationEvent::NetFloatingError&)
        {
        },
        [](SimulationEvent::Stopped&)
        {
        },
        [](SimulationEvent::OutputSocketsConflict&)
        {
        },
        [](SimulationEvent::FloatingInput&)
        {
        },
        [](SimulationEvent::RequestedBadScene&)
        {
        },
        [](SimulationEvent::SimulationError&)
        {
        },
        [&](SimulationEvent::Success& e)
        {
            m_last_simulation_result = std::move(e.result);
        }
        }, evt.e);
    m_end_callback(evt);
    m_current_running_simulator = nullptr;
}
