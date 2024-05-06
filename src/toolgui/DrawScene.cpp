#include "DrawScene.h"
#include <algorithm>
#include "util_drawing.h"
#include "toolgui/NodeMacros.h"

DrawScene* DrawScene::OnGetInteractableAtPoint(const SDL_Point &point)
{
    UNUSED_PARAM(point);
    return this;
}

DrawScene::DrawScene()
{
    this->AddSquare(200, 200, 50);
}

void DrawScene::AddSquare(int left, int top, int width)
{
    this->m_shapes.push_back({left, top, width, width});
}

void DrawScene::SetRect(const SDL_Rect &rect)
{
        this->m_rect = rect;
}

void DrawScene::Draw(SDL_Renderer *renderer)
{
    SDL_RenderSetClipRect(renderer, &(this->m_rect));
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    for (auto& shape: m_shapes)
    {
        SDL_Rect draw_rect{this->m_rect.x + shape.rect.x,
                           this->m_rect.y + shape.rect.y,
                           shape.rect.w, shape.rect.h};
        SDL_RenderFillRect(renderer, &draw_rect);
    }
    //SDL_Point center{500, 500};
    //draw_circle_filled(center, 100, renderer);
    SDL_RenderSetClipRect(renderer, nullptr);
}

SDL_Point DrawScene::ScreenToSceneCoords(const SDL_Point& point)
{
    return {point.x - this->m_rect.x,
                           point.y - this->m_rect.y};
}

void DrawScene::OnMouseMove(const SDL_Point &current_mouse_point)
{
    if (this->b_is_dragging)
    {
        if (SDL_PointInRect(&current_mouse_point, &(this->m_rect)))
        {
            Square& current_shape = this->m_shapes[this->m_current_dragged_shape];
            const SDL_Point& current_point = ScreenToSceneCoords(current_mouse_point); 
            current_shape.rect.x = current_point.x - this->m_shape_drag_start_point.x;
            current_shape.rect.y = current_point.y - this->m_shape_drag_start_point.y;
        }
    }
}

MI::ClickEvent DrawScene::OnLMBDown(const SDL_Point &current_mouse_point)
{
    const SDL_Point& screen_coord = ScreenToSceneCoords(current_mouse_point);
    auto it = std::find_if(this->m_shapes.rbegin(), this->m_shapes.rend(), [&](const Square& sqr)
    {
        return SDL_PointInRect(&screen_coord, &sqr.rect);
    });
    if (it != this->m_shapes.rend())
    {
        this->b_is_dragging = true;
        this->m_current_dragged_shape = static_cast<int>(std::distance(this->m_shapes.begin(),(it+1).base()));
        this->m_shape_drag_start_point = {screen_coord.x - (*it).rect.x , screen_coord.y - (*it).rect.y};
        return MI::ClickEvent::CAPTURE_START;
    }
    else
    {
        return MI::ClickEvent::NONE;
    }
}

MI::ClickEvent DrawScene::OnLMBUp(const SDL_Point &current_mouse_point)
{
    UNUSED_PARAM(current_mouse_point);
    if (b_is_dragging)
    {
        this->b_is_dragging = false;
        return MI::ClickEvent::CAPTURE_END;
    }
    else
    {
        return MI::ClickEvent::NONE;
    }
    
}

