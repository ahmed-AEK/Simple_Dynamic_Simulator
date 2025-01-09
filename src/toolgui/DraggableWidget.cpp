#include "DraggableWidget.hpp"
#include "Scene.hpp"
#include "toolgui/ContextMenu.hpp"
#include "SDL_Framework/SDL_Math.hpp"

node::DragableWidget::DragableWidget(const WidgetSize& size, node::Widget* parent)
: node::Widget(size, parent)
{

}

void node::DragableWidget::OnMouseMove(MouseHoverEvent& )
{
}

MI::ClickEvent node::DragableWidget::OnLMBDown(MouseButtonEvent&)
{
    return MI::ClickEvent::CAPTURE_START;
}

MI::ClickEvent node::DragableWidget::OnLMBUp(MouseButtonEvent& )
{
    return MI::ClickEvent::CAPTURE_END;
}