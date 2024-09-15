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

void node::SimulationManager::RunSimulator(model::NodeSceneModel& scene_model, std::shared_ptr<BlockClassesManager> classes_manager, Application& app)
{
    if (!m_current_running_simulator)
    {
        auto runner = std::make_shared<SimulatorRunner>(
            scene_model,
            std::move(classes_manager),
            std::function<void()>{
            [app = &app, this] { app->AddMainThreadTask([this]() { this->CheckSimulatorEnded(); }); } },
            m_simulationSettings);
        m_current_running_simulator = runner;
        runner->Run();
    }
}

void node::SimulationManager::StopSimulator()
{
    if (m_current_running_simulator)
    {
        m_current_running_simulator->Stop();
    }
}

void node::SimulationManager::CheckSimulatorEnded()
{
    if (m_current_running_simulator && m_current_running_simulator->IsEnded())
    {
        if (auto result = m_current_running_simulator->GetResult())
        {
            OnSimulationEnd(*result);
        }
        else
        {
            assert(false); // simulation ended but no result !!
        }

    }
}

void node::SimulationManager::SetSimulationEndCallback(std::function<void(const SimulationEvent&)> end_callback)
{
    m_end_callback = std::move(end_callback);
}

void node::SimulationManager::OnSimulationEnd(SimulationEvent& evt)
{
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
        [&](SimulationEvent::Success& e)
        {
            m_last_simulation_result = std::move(e.result);
        }
        }, evt.e);
    m_end_callback(evt);
    m_current_running_simulator = nullptr;
}