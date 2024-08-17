#include <iostream>
#include "Application.hpp"
#include "SDL_Framework/SDL_headers.h"
#include "toolgui/Scene.hpp"
#include <cassert>
#include <fstream>

namespace node
{
    Application::Application(int width, int height, std::string title)
    :m_title(title), m_framework(), m_width(width), m_height(height), m_rect_base{0,0, width, height},
        m_rect{ 0,0,width,height }
    {
        this->OnInit();
    }

    Application::~Application() = default;

    int Application::Run()
    {

        if (!m_framework.Init(SDL_INIT_VIDEO))
        {
            assert(false);
            return -1;
        }

#if defined linux && SDL_VERSION_ATLEAST(2, 0, 8)
        // Disable compositor bypass
        if(!SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0"))
        {
            SDL_Log("SDL can not disable compositor bypass!");
            assert(false);
            return -1;
        }
#endif
        SDL_SetHint(SDL_HINT_MOUSE_AUTO_CAPTURE, "0");
        // Create window
        m_window.reset(SDL_CreateWindow(m_title.c_str(),
                                            SDL_WINDOWPOS_UNDEFINED,
                                            SDL_WINDOWPOS_UNDEFINED,
                                            m_width, m_height,
                                            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE));
        if(!m_window)
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
        m_appFont = TTFFont{ TTF_OpenFont("./assets/FreeSans.ttf", 24) };
        if (!m_appFont)
        {
            SDL_Log("Failed to load Font \"./assets/FreeSans.ttf\"");
        }
        b_running = true;

        this->OnRun();
        // Event loop
        while(b_running)
        {
            this->HandleInputs();
            SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
            SDL_RenderClear(m_renderer);
            SDL_assert(m_scene);
            m_scene->Draw(m_renderer);
            // Update screen
            SDL_RenderPresent(m_renderer);
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
                case SDL_QUIT:
                {
                    b_running = false;
                    return true;
                }
                case SDL_MOUSEBUTTONDOWN:
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
                case SDL_MOUSEBUTTONUP:
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
                case SDL_MOUSEWHEEL:
                {
                    this->HandleMouseScroll(e);
                    return true;
                    break;
                }
                case SDL_MOUSEMOTION:
                {
                    this->HandleMouseMotion(e);
                    return true;
                }
                case SDL_KEYUP:
                {
                    if (SDL_SCANCODE_Q == e.key.keysym.scancode)
                    {
                        b_running = false;
                        return true;
                    };
                    break;
                }
                case SDL_WINDOWEVENT:
                {
                    if (SDL_WINDOWEVENT_RESIZED == e.window.event) {
                        SDL_Rect rect(0,0, e.window.data1, e.window.data2);
                        this->m_rect = rect;
                        assert(m_scene);
                        this->m_scene->SetRect(rect);
                        b_redrawScene = true;
                        return true;
                    }
                }
            }
            return false;
    }
    void Application::HandleInputs()
    {
        SDL_Event e;
        if (m_scene == nullptr || m_scene->UpdateTasksEmpty())
        {
            SDL_WaitEvent(&e);
            HandleEvent(e);
        }
        while (SDL_PollEvent(&e))
        {
            HandleEvent(e);
        }
        if (m_scene && !m_scene->UpdateTasksEmpty())
        {
            m_scene->DoUpdateTasks();
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
            m_scene->SetRect(m_rect);
        }
        m_scene->Start();
    }
    void Application::OnRun()
    {
        assert(m_scene);
        if (m_scene)
        {
            m_scene->OnInit();
            m_scene->SetRect(m_rect);
        }
        
    }
    void Application::HandleLMBDown(SDL_Event& e)
    {
        m_scene->LMBDown({e.button.x,e.button.y});
    }
    void Application::HandleRMBDown(SDL_Event& e)
    {
        m_scene->RMBDown({e.button.x,e.button.y});
    }
    void Application::HandleLMBUp(SDL_Event& e)
    {
        m_scene->LMBUp({e.button.x,e.button.y});
    }
    void Application::HandleRMBUp(SDL_Event& e)
    {
        m_scene->RMBUp({e.button.x,e.button.y});
    }
    void Application::HandleMouseMotion(SDL_Event& e)
    {
        m_scene->MouseMove({e.button.x,e.button.y});
    }
    void Application::HandleMouseScroll(SDL_Event& e)
    {
        SDL_Point p;
        SDL_GetMouseState(&p.x, &p.y);
        p = convert_to_renderer_coordinates(p.x, p.y);
        m_scene->Scroll(e.wheel.preciseY, p);
    }

    const SDL_Rect& Application::getRect() const
    {
        return m_rect;
    }
    void Application::InvalidateRect()
    {
        b_redrawScene = true;
    }

    SDL_Point Application::convert_to_renderer_coordinates(int x, int y)
    {
        SDL_Rect viewport;
        float scale_x, scale_y;
        SDL_RenderGetViewport(m_renderer, &viewport);
        SDL_RenderGetScale(m_renderer, &scale_x, &scale_y);
        return {static_cast<int>((x / scale_x) - viewport.x),
        static_cast<int>((y / scale_y) - viewport.y)};
    }
}
