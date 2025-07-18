#include <iostream>
#include "Application.hpp"
#include "SDL_Framework/SDL_headers.h"
#include "toolgui/Scene.hpp"
#include <cassert>
#include <fstream>
#include "SDL_Framework/Utility.hpp"
#include "Widget.hpp"

#include "AssetsManager/AssetsManager.hpp"

namespace node
{
    Application::Application(int width, int height, std::string title)
    :m_title(title), m_framework(), m_width(width), m_height(height),
        m_rect{ 0,0,width,height }
    {
    }

    Application::~Application() = default;

    int Application::Run()
    {
        Initialize();
        
        // Event loop
        while(b_running)
        {
            this->HandleInputs();
            Update();
        }
        return 0;
    }

    void Application::Stop()
    {
        this->b_running = false;
    }

    bool Application::HandleEvent(SDL_Event& e)
    {
            // User requests quit
            switch(e.type)
            {
                case SDL_EVENT_RENDER_DEVICE_RESET:
                case SDL_EVENT_RENDER_TARGETS_RESET:
                {
                    textures::ResetAllTextures();
                    break;
                }
                case SDL_EVENT_QUIT:
                {
                    b_running = false;
                    return true;
                }
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                {
                    if (SDL_BUTTON_LEFT == e.button.button)
                    {
                        this->HandleLMBDown(e);
                        return true;
                    }
                    else if (SDL_BUTTON_RIGHT == e.button.button)
                    {
                        this->HandleRMBDown(e);
                        return true;
                    }
                    break;
                }
                case SDL_EVENT_MOUSE_BUTTON_UP:
                {
                    if (SDL_BUTTON_LEFT == e.button.button)
                    {
                        this->HandleLMBUp(e);
                        return true;
                    }   
                    else if (SDL_BUTTON_RIGHT == e.button.button)
                    {
                        this->HandleRMBUp(e);
                        return true;
                    }
                    break;
                }
                case SDL_EVENT_MOUSE_WHEEL:
                {
                    this->HandleMouseScroll(e);
                    return true;
                }
                case SDL_EVENT_MOUSE_MOTION:
                {
                    this->HandleMouseMotion(e);
                    return true;
                }
                case SDL_EVENT_KEY_DOWN:
                case SDL_EVENT_KEY_UP:
                {
                    if (SDL_SCANCODE_BACKSPACE == e.key.scancode || 
                        SDL_SCANCODE_DELETE == e.key.scancode ||
                        SDL_SCANCODE_RETURN == e.key.scancode || 
                        SDL_SCANCODE_KP_ENTER == e.key.scancode ||
                        SDL_SCANCODE_LEFT == e.key.scancode || 
                        SDL_SCANCODE_UP == e.key.scancode ||
                        SDL_SCANCODE_DOWN == e.key.scancode ||
                        SDL_SCANCODE_RIGHT == e.key.scancode ||
                        SDL_SCANCODE_RSHIFT == e.key.scancode ||
                        SDL_SCANCODE_LSHIFT == e.key.scancode ||
                        SDL_SCANCODE_RCTRL == e.key.scancode ||
                        SDL_SCANCODE_LCTRL == e.key.scancode ||
                        SDL_SCANCODE_ESCAPE == e.key.scancode ||
                        SDL_SCANCODE_Z == e.key.scancode ||
                        SDL_SCANCODE_C == e.key.scancode ||
                        SDL_SCANCODE_V == e.key.scancode ||
                        SDL_SCANCODE_X == e.key.scancode ||
                        SDL_SCANCODE_A == e.key.scancode
                        )
                    {
                        if (e.key.scancode == SDL_SCANCODE_ESCAPE)
                        {
                            m_scene->CancelCurrentDrag();
                        }
                        m_scene->SendKeyPress({ e.key });
                        return true;
                    }
                    break;
                }
                case SDL_EVENT_TEXT_INPUT:
                {
                    m_scene->SendCharPress({ e.text });
                    return true;
                }
                case SDL_EVENT_WINDOW_RESIZED:
                {
                    SDL_Rect rect(0,0, e.window.data1, e.window.data2);
                    this->m_rect = rect;
                    assert(m_scene);
                    this->m_scene->SetSize({ static_cast<float>(rect.w), 
                        static_cast<float>(rect.h)});
                    b_redrawScene = true;
                    return true;
                }
                case SDL_EVENT_USER:
                {
                    if (e.user.code == 1)
                    {
                        HandleMainThreadTasks();
                    }
                    if (e.user.code == 2)
                    {
                        MakeTaskReady(std::coroutine_handle<Task::promise>::from_address(e.user.data1));
                    }
                    return true;
                }
            }
            return false;
    }
    void Application::HandleInputs()
    {
        SDL_Event e;
        if (m_scene == nullptr || UpdateTasksEmpty())
        {
            SDL_PollEvent(&e);
            HandleEvent(e);
        }
        while (SDL_PollEvent(&e))
        {
            HandleEvent(e);
        }
    }

    void Application::SetScene(std::unique_ptr<Scene> scene)
    {
        OnSetScene(scene);
    }

    void Application::OnSetScene(std::unique_ptr<Scene>& scene)
    { 
        m_scene = std::move(scene);
        if (b_running)
        {
            m_scene->SetSize({ static_cast<float>(m_rect.w),
                        static_cast<float>(m_rect.h) });
        }
        m_scene->Start();
    }
    void Application::OnRun()
    {
        assert(m_scene);
        if (m_scene)
        {
            m_scene->OnInit();
            m_scene->SetSize({ static_cast<float>(m_rect.w),
                        static_cast<float>(m_rect.h) });
        }
        
    }
    void Application::HandleLMBDown(SDL_Event& e)
    {
        m_scene->LMBDown(Widget::MouseButtonEvent{ e.button });
    }
    void Application::HandleRMBDown(SDL_Event& e)
    {
        m_scene->RMBDown(Widget::MouseButtonEvent{ e.button });
    }
    void Application::HandleLMBUp(SDL_Event& e)
    {
        m_scene->LMBUp(Widget::MouseButtonEvent{ e.button });
    }
    void Application::HandleRMBUp(SDL_Event& e)
    {
        m_scene->RMBUp(Widget::MouseButtonEvent{ e.button });
    }
    void Application::HandleMouseMotion(SDL_Event& e)
    {
        m_scene->MouseMove(Widget::MouseHoverEvent{ e.motion });
    }
    void Application::HandleMouseScroll(SDL_Event& e)
    {
        SDL_FPoint p;
        SDL_GetMouseState(&p.x, &p.y);
        p = convert_to_renderer_coordinates(p.x, p.y);
        m_scene->Scroll(e.wheel.y, p);
    }

    const SDL_Rect& Application::getRect() const
    {
        return m_rect;
    }
    void Application::InvalidateRect()
    {
        b_redrawScene = true;
    }


    void Application::DoUpdateTasks()
    {
        for (auto&& [key, value] : m_new_updateTasks)
        {
            m_updateTasks.emplace(key, std::move(value));
        }
        m_new_updateTasks.clear();

        for (auto&& id : m_deleted_updateTasks)
        {
            auto it = m_updateTasks.find(id);
            assert(it != m_updateTasks.end());
            if (it != m_updateTasks.end())
            {
                m_updateTasks.erase(it);
            }
        }
        m_deleted_updateTasks.clear();

        for (auto&& task : m_updateTasks)
        {
            if (task.second.isAlive())
            {
                task.second.task();
            }
            else
            {
                m_deleted_updateTasks.push_back(task.first);
            }
        }
    }

    int64_t Application::AddUpdateTask(UpdateTask task)
    {
        auto current_task = m_current_task_id;
        m_new_updateTasks.emplace(current_task, std::move(task));
        m_current_task_id++;
        return current_task;
    }

    void Application::RemoveUpdateTask(int64_t task_id)
    {
        m_deleted_updateTasks.push_back(task_id);
    }

    void Application::StartTextInput(const SDL_Rect& rect)
    {
        SDL_SetTextInputArea(m_window.get(), &rect, 0);
        SDL_StartTextInput(m_window.get());
    }

    void Application::StopTextInput()
    {
        SDL_StopTextInput(m_window.get());
    }

    int Application::Initialize()
    {
        m_async_eventLoop.SetThreadAffinity();
        if (!m_framework.Init(SDL_INIT_VIDEO))
        {
            assert(false);
            return -1;
        }

#if defined linux && SDL_VERSION_ATLEAST(2, 0, 8)
        // Disable compositor bypass
        if (!SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0"))
        {
            SDL_Log("SDL can not disable compositor bypass!");
            assert(false);
            return -1;
        }
#endif
        SDL_SetHint(SDL_HINT_MOUSE_AUTO_CAPTURE, "0");
        // Create window
        m_window.reset(SDL_CreateWindow(m_title.c_str(),
            m_width, m_height,
            SDL_WINDOW_RESIZABLE));
        if (!m_window)
        {
            SDL_Log("Window could not be created!");
            std::cout << "SDL_Error: " << SDL_GetError() << std::endl;
            assert(false);
            return -1;
        }

        // Create renderer
        if (!m_renderer.Init(m_window.get()))
        {
            SDL_Log("Failed to initialize Renderer.");
            assert(false);
            return -1;
        }

        // load Font
        AssetsManager manager;
        const char* font_path = "assets/Roboto-Regular.ttf";
        auto font_resource = manager.GetResource(font_path);
        if (!font_resource)
        {
            SDL_Log("failed to load font resource %s", font_path);
            return -1;
        }
        auto font_ops1 = SDL_IOFromConstMem(font_resource->data(), static_cast<int>(font_resource->size()));
        m_appFonts[0] = TTFFont{ TTF_OpenFontIO(font_ops1, 1, 20) };
        if (!m_appFonts[0])
        {
            SDL_Log("Failed to load Font %s", font_path);
        }
        auto font_ops2 = SDL_IOFromConstMem(font_resource->data(), static_cast<int>(font_resource->size()));
        m_appFonts[1] = TTFFont{ TTF_OpenFontIO(font_ops2, 1, 16) };
        if (!m_appFonts[1])
        {
            SDL_Log("Failed to load Font %s", font_path);
        }

        b_running = true;

        this->OnRun();
        return 0;
    }

    int Application::Update()
    {
        if (m_scene && !UpdateTasksEmpty())
        {
            DoUpdateTasks();
        }
        m_async_eventLoop.DispatchReadyTasks();
        SDL_assert(m_scene);

        OnUpdateBegin();

        const SDL_Color BG_Color = m_scene->GetBGColor();
        SDL_SetRenderDrawColor(m_renderer, BG_Color.r, BG_Color.g, BG_Color.b, BG_Color.a);
        SDL_RenderClear(m_renderer);
        m_scene->Draw(m_renderer);
        // Update screen
        SDL_RenderPresent(m_renderer);

        if (UpdateTasksEmpty())
        {
            m_framerateController.SetControlType(FrameRateController::ControlType::Wait);
        }
        else
        {
            m_framerateController.SetControlType(FrameRateController::ControlType::Uncapped);
        }
        m_framerateController.Apply();
        return 0;
    }
    
    void Application::AddMainThreadTask(std::function<void()> task)
    {
        m_mainThreadTasks.Push(std::move(task));
        SDL_Event event;
        event.type = SDL_EVENT_USER;
        event.user.type = SDL_EVENT_USER;
        event.user.timestamp = SDL_GetTicks();
        event.user.code = 1;
        SDL_PushEvent(&event);
    }

    void Application::HandleMainThreadTasks()
    {
        std::optional<std::function<void()>> func;
        while ((func = m_mainThreadTasks.TryPop()))
        {
            (*func)();
        }
    }

    void Application::DispatchTask(async::Task task)
    {
        m_async_eventLoop.Dispatch(std::move(task));
    }
    void Application::PostTask(async::Task task)
    {
        m_async_eventLoop.Post(std::move(task));
    }
    void Application::MakeTaskReady(std::coroutine_handle<Task::promise> h)
    {
        m_async_eventLoop.MakeReady(h);
    }

    SDL_FPoint Application::convert_to_renderer_coordinates(float x, float y)
    {
        SDL_Rect viewport;
        float scale_x, scale_y;
        SDL_GetRenderViewport(m_renderer, &viewport);
        SDL_GetRenderScale(m_renderer, &scale_x, &scale_y);
        return {(x / scale_x) - viewport.x,(y / scale_y) - viewport.y};
    }

    void FrameRateController::Apply()
    {
        if (m_current_control == m_current_set_control)
        {
            return;
        }
        if (m_current_control == ControlType::Wait)
        {
            SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, "waitevent");
        }
        else
        {
            SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, "0");
        }
        m_current_set_control = m_current_control;

    }
}


void node::TaskQueue::Push(std::function<void()> func)
{
    std::lock_guard g{ m_mutex };
    m_tasks.push(std::move(func));
}

std::optional<std::function<void()>> node::TaskQueue::TryPop()
{
    std::optional<std::function<void()>> ret_val = std::nullopt;
    {
        std::lock_guard g{ m_mutex };
        if (m_tasks.empty())
        {
            return ret_val;
        }
        ret_val = m_tasks.front();
        m_tasks.pop();
    }
    return ret_val;
}

bool node::TaskQueue::IsEmpty()
{
    std::lock_guard g{ m_mutex };
    return m_tasks.empty();
}

node::UpdateTask node::UpdateTask::FromWidget(Widget& widget, std::function<void()> task)
{
    return UpdateTask{ [handle = widget.GetMIHandlePtr()]() {return handle.isAlive(); }, std::move(task) };
}


node::Application::ClipboardString node::Application::GetClipboardText()
{
    if (!SDL_HasClipboardText())
    {
        return ClipboardString{};
    }
    char* clipboard_text = SDL_GetClipboardText();
    if (!clipboard_text)
    {
        return ClipboardString{};
    }
    return ClipboardString{ clipboard_text };
}

bool node::Application::SetClipboardText(const std::string& text)
{
    if (SDL_SetClipboardText(text.c_str()))
    {
        m_logger.LogDebug("copied {} characters to clipboard", text.size());
        return true;
    }
    m_logger.LogDebug("failed to copy text to clipboard");
    return false;
}

node::Application::ClipboardString::ClipboardString()
{
}

node::Application::ClipboardString::ClipboardString(char* str)
    :m_str{ str }, m_size{ str ? strlen(str) : 0}
{
}

node::Application::ClipboardString::ClipboardString(ClipboardString&& other) noexcept
    :m_str{std::exchange(other.m_str, nullptr)}, m_size{std::exchange(other.m_size, 0)}
{
}

node::Application::ClipboardString& node::Application::ClipboardString::operator=(ClipboardString&& other) noexcept
{
    std::ranges::swap(m_str, other.m_str);
    std::ranges::swap(m_size, other.m_size);
    return *this;
}

node::Application::ClipboardString::~ClipboardString()
{
    SDL_free(static_cast<void*>(m_str));
}

std::string_view node::Application::ClipboardString::view() const
{
    return std::string_view{ m_str, m_size };
}

void node::Application::DispatchThreadedTaskInner(std::function<void()> func, 
    async::ThreadedPromise<std::monostate>& p)
{
    async::Future<std::monostate> fut = p.get_future();
    std::jthread t([](std::function<void()> f, async::ThreadedPromise<std::monostate>* p) {
        f();
        p->set_value(std::monostate{});
        }, func, &p);

    DispatchTask([](async::Future<std::monostate> fut, std::jthread t) ->Task { 
        co_await std::move(fut);  
        t.join(); 
        }(std::move(fut), std::move(t)));
}