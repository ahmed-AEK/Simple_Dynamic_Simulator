#include "SDL2/SDL_ttf.h"
#include "toolgui/ContextMenu.hpp"
#include "SDL_Framework/SDL_headers.h"
#include "toolgui/Scene.hpp"

node::ContextMenu::ContextMenu(node::Scene* parent)
: Widget({0,0,0,0},parent), m_font(TTFFont(TTF_OpenFont("assets/FreeSans.ttf", 24)))
{
    SDL_assert(m_font.get());
    int other;
    TTF_SizeText(m_font.get(), "A", &other, &m_element_height);
    
}

bool node::ContextMenu::OnElementSelected(uint64_t element)
{
    SDL_Log("context item selected = %d", static_cast<int>(element));
    return true;
}


void node::ContextMenu::Draw(SDL_Renderer* renderer)
{
    SDL_SetRenderDrawColor(renderer, 42, 46, 50, 255);
    SetRect({ GetRect().x, GetRect().y, 100, static_cast<int>(m_items.size() * m_element_height) });
    SDL_RenderFillRect(renderer, &GetRect());
    SDL_Rect inside_rect{ GetRect().x +2, GetRect().y + 2, GetRect().w - 4, GetRect().h-4};
    SDL_SetRenderDrawColor(renderer, 242, 242, 242, 255);
    SDL_RenderFillRect(renderer, &inside_rect);
    int position = 0;
    for (auto&& [key,value]: m_items)
    {
        auto iter = m_fonts.find(key);
        if (iter == m_fonts.end())
        {
            SDL_Color Black = {0,0,0,255};
            SDLSurface surface = SDLSurface(TTF_RenderText_Solid(m_font.get(), key.c_str(), Black));
            iter = m_fonts.insert(std::make_pair(key,std::move(surface))).first;
        }
        SDLTexture tex = SDLTexture(SDL_CreateTextureFromSurface(renderer, iter->second.get()));
        SDL_Rect target_rect{GetRect().x + 4, GetRect().y + m_element_height * position,
        iter->second.get()->w, 
        iter->second.get()->h};
        SDL_RenderCopy(renderer, tex.get(), nullptr, &target_rect);
        position++;
    }
    
}

void node::ContextMenu::OnMouseMove(const SDL_Point& current_mouse_point)
{
    UNUSED_PARAM(current_mouse_point);
}

MI::ClickEvent node::ContextMenu::OnLMBDown(const SDL_Point& current_mouse_point)
{
    int clicked_item = (current_mouse_point.y - GetRect().y) / m_element_height;
    element_being_clicked = clicked_item;
    return MI::ClickEvent::NONE;
}

MI::ClickEvent node::ContextMenu::OnLMBUp(const SDL_Point& current_mouse_point)
{
    int clicked_item = (current_mouse_point.y - GetRect().y) / m_element_height;
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
        GetScene()->DestroyContextMenu();
        return MI::ClickEvent::CLICKED;
    }
    return MI::ClickEvent::NONE;
    
}