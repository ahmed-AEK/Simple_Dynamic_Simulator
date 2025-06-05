#include "NodeEditorApp.hpp"
#include "NodeEditorApp/MainNodeScene.hpp"
#include "NodeEditorApp/NodeGraphicsScene.hpp"
#include "GraphicsScene/ToolsManager.hpp"
#include "SDL_Framework/Utility.hpp"
#include <iostream>

namespace node
{
    NodeEditorApplication::NodeEditorApplication()
    : Application(800,600, "Dynamic Simulator"), m_logger{this}
    {
        std::unique_ptr<MainNodeScene> scene = std::make_unique<MainNodeScene>(
            WidgetSize{ static_cast<float>(getRect().w), static_cast<float>(getRect().h) }, this);
        m_scene = scene.get();
        SetScene(std::move(scene));
    }
    NodeEditorApplication::~NodeEditorApplication()
    {
        SDL_SetLogOutputFunction(SDL_GetDefaultLogOutputFunction(), nullptr);
    }
    void NodeEditorApplication::OnRun()
    {
        m_logger.SetDefaultLogger(SDL_GetDefaultLogOutputFunction());
        SDL_SetLogOutputFunction(m_logger.LogSDLMessage, &m_logger);
#ifndef NDEBUG
        SDL_SetLogPriorities(SDL_LOG_PRIORITY_DEBUG);
#else  // NDEBUG
        SDL_SetLogPriorities(SDL_LOG_PRIORITY_INFO);
#endif // NDEBUG

        Application::OnRun();
        m_logger.SetLogView(m_scene->GetLogView());
        m_logger.SetOnError([this] {this->m_scene->OpenBotPanel(); });
    }
    void NodeEditorApplication::OnUpdateBegin()
    {
        m_logger.DispatchLogs();
    }
}
