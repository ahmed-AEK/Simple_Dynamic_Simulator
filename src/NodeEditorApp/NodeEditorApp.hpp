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
        MainNodeScene* GetScene() noexcept;
    private:
        MainNodeScene* m_scene;
    };
}