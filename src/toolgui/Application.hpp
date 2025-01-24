#pragma once

#include "toolgui/toolgui_exports.h"

#include "SDL_Framework/SDLFramework.hpp"
#include "SDL_Framework/SDLRenderer.hpp"
#include "SDL_Framework/SDLCPP.hpp"

#include <memory>
#include <string>
#include <functional>
#include <optional>
#include <queue>
#include <mutex>
#include <array>

namespace node
{

class Scene;
class Widget;

class TaskQueue
{
public:
    void Push(std::function<void()> func);
    std::optional<std::function<void()>> TryPop();
    bool IsEmpty();
private:
    std::queue<std::function<void()>> m_tasks;
    std::mutex m_mutex;
};

struct TOOLGUI_API UpdateTask
{
    std::function<bool()> isAlive;
    std::function<void()> task;
    static UpdateTask FromWidget(Widget& widget, std::function<void()> task);
};

enum class FontType : char
{
    Title,
    Label
};

class FrameRateController
{
public:
    enum class ControlType: char
    {
        None,
        Wait,
        Uncapped
    };

    void Apply();
    void SetControlType(ControlType control)
    {
        m_current_control = control;
    }
private:
    ControlType m_current_control{ ControlType::None };
    ControlType m_current_set_control{ ControlType::None };
};
class TOOLGUI_API Application
{
public:
    Application(int width, int height, std::string title = "Basic Application");
    Application(Application&) = delete;
    Application& operator=(Application&) = delete;
    int Run();
    void Stop();
    void SetScene(std::unique_ptr<Scene> scene);
    virtual ~Application();
    const SDL_Rect& getRect() const;
    void InvalidateRect();
    const TTFFont& getFont(FontType type = FontType::Title) const { return m_appFonts[static_cast<int>(type)]; }

    void AddMainThreadTask(std::function<void()> task);
    void HandleMainThreadTasks();

    bool UpdateTasksEmpty() { return m_updateTasks.empty() && m_new_updateTasks.empty(); }
    void DoUpdateTasks();
    int64_t AddUpdateTask(UpdateTask task);
    void RemoveUpdateTask(int64_t task_id);
    Scene* GetScene() { return m_scene.get(); }
    void StartTextInput();
    void StopTextInput();
    int Initialize();
    int Update();
    bool virtual HandleEvent(SDL_Event& e);
    bool IsRunning() const { return b_running; }
    SDL_Window* GetWindow() const { return m_window.get(); }
protected:
    virtual void OnInit() {};
    virtual void OnRun();
    virtual void HandleLMBDown(SDL_Event& e);
    virtual void HandleRMBDown(SDL_Event& e);
    virtual void HandleLMBUp(SDL_Event& e);
    virtual void HandleRMBUp(SDL_Event& e);
    virtual void HandleMouseMotion(SDL_Event& e);
    virtual void HandleMouseScroll(SDL_Event& e);
    
    virtual void OnSetScene(std::unique_ptr<Scene>& scene);
    SDL_FPoint convert_to_renderer_coordinates(float x, float y);
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
    std::array<TTFFont,2> m_appFonts;

    std::unordered_map<int64_t, UpdateTask> m_updateTasks;
    std::unordered_map<int64_t, UpdateTask> m_new_updateTasks;
    std::vector<int64_t> m_deleted_updateTasks;
    int64_t m_current_task_id = 1;
    TaskQueue m_mainThreadTasks;
    FrameRateController m_framerateController;
};
}