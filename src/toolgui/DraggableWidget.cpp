#include "DraggableWidget.hpp"
#include "Scene.hpp"
#include "toolgui/ContextMenu.hpp"

node::DragableWidget::DragableWidget(const SDL_Rect& rect, node::Scene* parent)
: node::Widget(rect, parent)
{

}

void node::DragableWidget::OnMouseMove(const SDL_Point& current_mouse_point)
{
    if (b_dragging)
    {
        SDL_Rect new_rect = GetRect();
        new_rect.x = current_mouse_point.x - m_dragStartPoint.x;
        new_rect.y = current_mouse_point.y - m_dragStartPoint.y;
        SetRect(new_rect);
        GetScene()->InvalidateRect();
    }
}
MI::ClickEvent node::DragableWidget::OnLMBDown(const SDL_Point& current_mouse_point)
{
    m_dragStartPoint = {current_mouse_point.x - GetRect().x, current_mouse_point.y - GetRect().y};
    b_dragging = true;
    return MI::ClickEvent::CAPTURE_START;
}

MI::ClickEvent node::DragableWidget::OnLMBUp(const SDL_Point& current_mouse_point)
{
    UNUSED_PARAM(current_mouse_point);
    b_dragging = false;
    return MI::ClickEvent::CAPTURE_END;
}