#pragma once

#include "toolgui/toolgui_exports.h"

#include "toolgui/Widget.hpp"

#include "SDL_Framework/SDLCPP.hpp"
#include <unordered_map>
#include <string>

namespace node
{

class TOOLGUI_API ContextMenu : public node::Widget
{
public:
    ContextMenu(node::Scene* parent);
    virtual void Draw(SDL_Renderer* renderer) override;
protected:
    void OnMouseMove(const SDL_Point& current_mouse_point) override;
    MI::ClickEvent OnLMBDown(const SDL_Point& current_mouse_point) override;
    MI::ClickEvent OnLMBUp(const SDL_Point& current_mouse_point) override;
    virtual bool OnElementSelected(uint64_t element);
    int m_element_height;
    std::unordered_map<std::string, SDLSurface> m_fonts;
    std::unordered_map<std::string, uint64_t> m_items;
    TTFFont m_font;
private:
    int element_being_clicked = 0;
};
}
