#include "toolgui/ContextMenu.hpp"
#include "SDL_Framework/SDL_headers.h"
#include "toolgui/Scene.hpp"
#include "Application.hpp"

node::ContextMenu::ContextMenu(node::Widget* parent, TTF_Font* font)
: Widget({0,0,0,0},parent), m_font{font}
{
    SDL_assert(m_font);
    int other;
    std::string_view s = "A";
    TTF_GetStringSize(m_font, s.data(), s.size(), &other, &m_element_height);
    
}

bool node::ContextMenu::OnElementSelected(uint64_t element)
{
    SDL_Log("context item selected = %d", static_cast<int>(element));
    return true;
}


void node::ContextMenu::Draw(SDL_Renderer* renderer)
{
    SDL_SetRenderDrawColor(renderer, 42, 46, 50, 255);
    SetRect({ GetRect().x, GetRect().y, 100, static_cast<float>(m_items.size() * m_element_height) });
    SDL_RenderFillRect(renderer, &GetRect());
    SDL_FRect inside_rect{ GetRect().x +2, GetRect().y + 2, GetRect().w - 4, GetRect().h-4};
    SDL_SetRenderDrawColor(renderer, 242, 242, 242, 255);
    SDL_RenderFillRect(renderer, &inside_rect);
    int position = 0;
    for (auto&& [key,value]: m_items)
    {
        auto iter = m_fonts.find(key);
        if (iter == m_fonts.end())
        {
            SDL_Color Black = {0,0,0,255};
            SDLSurface surface = SDLSurface(TTF_RenderText_Blended(m_font, key.c_str(), key.size(), Black));
            iter = m_fonts.insert(std::make_pair(key,std::move(surface))).first;
        }
        SDLTexture tex = SDLTexture(SDL_CreateTextureFromSurface(renderer, iter->second.get()));
        SDL_FRect target_rect{GetRect().x + 4, GetRect().y + m_element_height * position,
        static_cast<float>(iter->second.get()->w), 
        static_cast<float>(iter->second.get()->h)};
        SDL_RenderTexture(renderer, tex.get(), nullptr, &target_rect);
        position++;
    }
    
}

void node::ContextMenu::OnMouseMove(MouseHoverEvent& e)
{
    UNUSED_PARAM(e);
}

MI::ClickEvent node::ContextMenu::OnLMBDown(MouseButtonEvent& e)
{
    SDL_FPoint current_mouse_point{ e.point() };
    int clicked_item = static_cast<int>((current_mouse_point.y - GetRect().y) / m_element_height);
    element_being_clicked = clicked_item;
    return MI::ClickEvent::NONE;
}

MI::ClickEvent node::ContextMenu::OnLMBUp(MouseButtonEvent& e)
{
    SDL_FPoint current_mouse_point{ e.point() };
    int clicked_item = static_cast<int>((current_mouse_point.y - GetRect().y) / m_element_height);
    if (clicked_item != element_being_clicked)
    {
        return MI::ClickEvent::NONE;
    }

    bool handled = false;
    int clicked_item_counter = 0;
    for (auto& [key,value]: m_items)
    {
        if (clicked_item_counter == clicked_item)
        {
            handled = OnElementSelected(value);
            break;
        }
        clicked_item_counter++;
    }
    if (handled)
    {
        GetApp()->GetScene()->DestroyContextMenu();
        return MI::ClickEvent::CLICKED;
    }
    return MI::ClickEvent::NONE;
    
}