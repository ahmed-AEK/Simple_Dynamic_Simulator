#include "DraggableWidget.hpp"
#include "Scene.hpp"
#include "toolgui/ContextMenu.hpp"

node::DragableWidget::DragableWidget(const SDL_Rect& rect, node::Widget* parent)
: node::Widget(rect, parent)
{

}

void node::DragableWidget::OnMouseMove(MouseHoverEvent& e)
{
    if (b_dragging)
    {
        SDL_Point current_mouse_point{ e.point() };
        SDL_Rect new_rect = GetRect();
        new_rect.x = current_mouse_point.x - m_dragStartPoint.x;
        new_rect.y = current_mouse_point.y - m_dragStartPoint.y;
        SetRect(new_rect);
    }
}
MI::ClickEvent node::DragableWidget::OnLMBDown(MouseButtonEvent& e)
{
    SDL_Point current_mouse_point{ e.point() };
    m_dragStartPoint = {current_mouse_point.x - GetRect().x, current_mouse_point.y - GetRect().y};
    b_dragging = true;
    return MI::ClickEvent::CAPTURE_START;
}

MI::ClickEvent node::DragableWidget::OnLMBUp(MouseButtonEvent& e)
{
    UNUSED_PARAM(e);
    b_dragging = false;
    return MI::ClickEvent::CAPTURE_END;
}