#pragma once

#include "toolgui/toolgui_exports.h"

#include "toolgui/Widget.hpp"

#include "SDL_Framework/SDLCPP.hpp"
#include <unordered_map>
#include <string>
#include "SDL_Framework/Utility.hpp"

namespace node
{

class TOOLGUI_API ContextMenu : public node::Widget
{
public:
    ContextMenu(node::Widget* parent, TTF_Font* font);
    virtual void OnDraw(SDL::Renderer& renderer) override;
protected:
    void OnMouseMove(MouseHoverEvent& e) override;
    MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;
    MI::ClickEvent OnLMBUp(MouseButtonEvent& e) override;
    virtual bool OnElementSelected(uint64_t element);
    int m_element_height;
    std::unordered_map<std::string, SDLSurface> m_fonts;
    std::unordered_map<std::string, uint64_t> m_items;
    TTF_Font* m_font;
private:
    int element_being_clicked = 0;
};
}
