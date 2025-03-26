#pragma once

#include "toolgui/Application.hpp"

namespace node
{
    class MainNodeScene;
    class MainNodeScene;
    class NodeEditorApplication : public node::Application
    {
    public:
        NodeEditorApplication();
        MainNodeScene* GetMainScene() { return m_scene; };
    private:
        MainNodeScene* m_scene;
    };
}