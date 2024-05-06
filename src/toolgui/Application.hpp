#pragma once

#include "toolgui/toolgui_exports.h"

#include "SDL_Framework/SDLFramework.hpp"
#include "SDL_Framework/SDLRenderer.hpp"
#include "SDL_Framework/SDLCPP.hpp"
#include "toolgui/Scene.hpp"
#include <memory>

namespace node
{
class TOOLGUI_API Application
{
public:
    Application(int width, int height, std::string title = "Basic Application");
    Application(Application&) = delete;
    Application& operator=(Application&) = delete;
    int Run();
    void Stop();
    void SetScene(std::unique_ptr<Scene> scene);
    virtual ~Application() noexcept {};
    const SDL_Rect& getRect() const;
    void InvalidateRect();
    const TTFFont& getFont() { return m_appFont; }
protected:
    virtual void OnInit() {};
    virtual void OnRun() {};
    virtual void HandleLMBDown(SDL_Event& e);
    virtual void HandleRMBDown(SDL_Event& e);
    virtual void HandleLMBUp(SDL_Event& e);
    virtual void HandleRMBUp(SDL_Event& e);
    virtual void HandleMouseMotion(SDL_Event& e);
    virtual void HandleMouseScroll(SDL_Event& e);
    
    virtual void OnSetScene(std::unique_ptr<Scene>& scene);
    bool virtual HandleEvent(SDL_Event& e);
    SDL_Point convert_to_renderer_coordinates(int x, int y);
    Scene* GetScene() { return m_scene.get(); }
private:
    void HandleInputs();
    std::string m_title;
    SDL::SDLFramework m_framework;
    SDLWindow m_window;
    SDL::Renderer m_renderer;
    std::unique_ptr<Scene> m_scene;
    int m_width;
    int m_height;
    SDL_Rect m_rect_base;
    SDL_Rect m_rect;
    bool b_running = false;
    bool b_dragging = false;
    bool b_redrawScene = true;
    TTFFont m_appFont;
};
}