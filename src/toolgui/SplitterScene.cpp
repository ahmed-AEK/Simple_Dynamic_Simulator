#include "SplitterScene.h"
#include "SDL_Framework/SDLRenderer.h"
#include "util_drawing.h"
#include "toolgui/NodeMacros.h"

void SplitterScene::SetRect(const SDL_Rect &rect)
{
    m_rect = rect;
}

void SplitterScene::Draw(SDL_Renderer* renderer)
{
    SDL_SetRenderDrawColor(renderer, m_color.r, m_color.g, m_color.b, m_color.a);
    SDL_Rect sash_rect = this->GetSashRect();
    SDL_RenderFillRect(renderer, &sash_rect);
    this->m_toolbar.Draw(renderer);
    this->m_scene.Draw(renderer);
}

void SplitterScene::SetSashPosition(int position)
{
    this->SashPosition = position;
    this->m_toolbar.SetRect({m_rect.x, m_rect.y, m_rect.w, this->SashPosition});
    int scene_start = m_rect.y + this->SashPosition + this->SashWidth;
    this->m_scene.SetRect({m_rect.x , scene_start, m_rect.w, m_rect.h - scene_start});
}

const SDL_Rect &SplitterScene::GetRect() const noexcept
{
    return m_rect;
}

SDL_Rect SplitterScene::GetSashRect() const noexcept
{
    return SDL_Rect{m_rect.x, m_rect.y + SashPosition, m_rect.w, SashWidth };
}

void SplitterScene::DragSash(const SDL_Point &p)
{
    this->SetSashPosition(std::min(p.y - this->m_rect.y,100));
}

SplitterScene* SplitterScene::OnGetInteractableAtPoint(const SDL_Point &point)
{
    SDL_Rect sash_rect = this->GetSashRect();
    if (SDL_PointInRect(&point,&sash_rect))
    {
        return this;
    }
    return this;
}

void SplitterScene::OnMouseMove(const SDL_Point &current_mouse_point)
{
    if (m_dragging)
    {
        DragSash(current_mouse_point);
    }
}

void SplitterScene::OnMouseIn()
{
    if (!m_resize_cursor)
    {
        m_resize_cursor.reset(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS));
    }
    SDL_SetCursor(m_resize_cursor.get());
}

void SplitterScene::OnMouseOut()
{
    SDL_SetCursor(SDL_GetDefaultCursor());
}

MI::ClickEvent SplitterScene::OnLMBDown(const SDL_Point &current_mouse_point)
{
    UNUSED_PARAM(current_mouse_point);
    this->m_dragging = true;
    return MI::ClickEvent::CAPTURE_START;
}

MI::ClickEvent SplitterScene::OnLMBUp(const SDL_Point &current_mouse_point)
{
    UNUSED_PARAM(current_mouse_point);
    this->m_dragging = false;
    return MI::ClickEvent::CAPTURE_END;
}
