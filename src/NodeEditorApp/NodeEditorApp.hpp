#pragma once

#include "toolgui/Application.hpp"

namespace node
{
    class MainNodeScene;

    class NodeEditorApplication : public node::Application
    {
    public:
        NodeEditorApplication();
        MainNodeScene* GetScene() noexcept;
    protected:
        bool HandleEvent(SDL_Event& e) override;
    };
}