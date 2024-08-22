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
    const TTFFont& getFont() { return m_appFont; }

    void AddMainThreadTask(std::function<void()> task);
    void HandleMainThreadTasks();

    bool UpdateTasksEmpty() { return m_updateTasks.empty() && m_new_updateTasks.empty(); }
    void DoUpdateTasks();
    int64_t AddUpdateTask(UpdateTask task);
    void RemoveUpdateTask(int64_t task_id);

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

    std::unordered_map<int64_t, UpdateTask> m_updateTasks;
    std::unordered_map<int64_t, UpdateTask> m_new_updateTasks;
    std::vector<int64_t> m_deleted_updateTasks;
    int64_t m_current_task_id = 1;
    TaskQueue m_mainThreadTasks;

};
}