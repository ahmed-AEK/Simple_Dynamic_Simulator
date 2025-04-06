#pragma once

#include "toolgui/Application.hpp"
#include "NodeEditorApp/LogHandler.hpp"

namespace node
{
    class MainNodeScene;
    class MainNodeScene;
    class NodeEditorApplication : public node::Application
    {
    public:
        NodeEditorApplication();
        MainNodeScene* GetMainScene() { return m_scene; };
        ~NodeEditorApplication();
    protected:
        void OnRun() override;
        void OnUpdateBegin() override;
    private:
        MainNodeScene* m_scene;
        LogHandler m_logger;
    };
}