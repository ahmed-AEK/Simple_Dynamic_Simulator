#pragma once

#include "toolgui/Widget.hpp"

#include "SDL_Framework/SDLCPP.hpp"
#include <functional>

namespace node
{
    class ButtonWidget: public Widget
    {
    public:
        ButtonWidget(const SDL_FRect& rect, std::string label, std::function<void(void)> action, node::Widget* parent);
        void Draw(SDL_Renderer* renderer) override;
    protected:
        virtual MI::ClickEvent OnLMBUp(MouseButtonEvent& e) override;
    private:
        constexpr static int w_margin = 5;
        constexpr static int h_margin = 5;
        std::string m_label;
        SDLSurface m_textSurface;
        SDLTexture m_textTexture;
        std::function<void(void)> m_action;
    };
}