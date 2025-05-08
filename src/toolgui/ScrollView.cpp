#include "ScrollView.hpp"
#include "SDL_Framework/SDLRenderer.hpp"
#include <cmath>

node::ScrollViewBase::ScrollViewBase(const WidgetSize& size, Widget* parent)
	:Widget{size, parent}, m_scrollbar{ GetScrollBarSize(), this }
{

}
node::ScrollView::ScrollView(const WidgetSize& size, Widget* parent)
	: ScrollViewBase{size, parent}
{
}

void node::ScrollView::SetWidget(std::unique_ptr<Widget> widget)
{
	if (m_contained_widget)
	{
		SetScrollInfo(0, 0, 0);
	}
	m_contained_widget = std::move(widget);
	if (m_contained_widget)
	{
		m_contained_widget->SetParent(this);
		m_contained_widget->SetPosition({ 0,0 });
		m_contained_widget->SetSize({ GetContainedAreaSize().w, m_contained_widget->GetSize().h });
		SetScrollInfo(GetSize().h, m_contained_widget->GetSize().h - GetSize().h, 0);
	}
}



void node::ScrollViewBase::RequestPosition(float new_position)
{
	if (!std::isfinite(new_position))
	{
		new_position = 0;
	}
	OnPositionRequested(new_position);
}

bool node::ScrollViewBase::OnScroll(const double amount, const SDL_FPoint&)
{
	auto new_value = m_scrollbar.GetScrollPosition() - amount * m_scroll_strength;
	RequestPosition(static_cast<float>(new_value));
	return true;
}

void node::ScrollViewBase::OnSetSize(const WidgetSize& size)
{
	Widget::OnSetSize(size);
	m_scrollbar.SetPosition(GetScrollBarPosition());
	m_scrollbar.SetSize(GetScrollBarSize());
}

void node::ScrollView::OnSetSize(const WidgetSize& size)
{
	ScrollViewBase::OnSetSize(size);
	if (m_contained_widget)
	{
		m_contained_widget->SetSize({ GetContainedAreaSize().w, m_contained_widget->GetSize().h });
		SetScrollInfo(GetSize().h, m_contained_widget->GetSize().h - GetSize().h, GetScrollInfo().position);
		if (auto bar_max_pos = GetScrollInfo().max_position; bar_max_pos < GetScrollInfo().position)
		{
			SetScrollPosition(bar_max_pos);
			m_contained_widget->SetPosition({ 0, -bar_max_pos });
		}
	}
}

void node::ScrollViewBase::OnPositionRequested(float new_position)
{
	float current_max = m_scrollbar.GetMaxPosition();
	float current_size = m_scrollbar.GetScrollSize();
	SetScrollInfo(current_size, current_max, new_position);
}

node::ScrollViewBase::ScrollInfo node::ScrollViewBase::GetScrollInfo() const
{
	float current_max = m_scrollbar.GetMaxPosition();
	float current_size = m_scrollbar.GetScrollSize();
	float current_position = m_scrollbar.GetScrollPosition();
	return ScrollInfo{ current_size, current_max, current_position };
}
void node::ScrollView::OnPositionRequested(float new_position)
{
	auto current_scroll_info = GetScrollInfo();
	if (new_position < 0)
	{
		new_position = 0;
	}
	if (auto bar_max_pos = current_scroll_info.max_position; bar_max_pos < new_position)
	{
		SetScrollPosition(bar_max_pos);
		m_contained_widget->SetPosition({ 0, -bar_max_pos });
	}
	else
	{
		SetScrollPosition(new_position);
		m_contained_widget->SetPosition({ 0, -new_position });
	}
}

node::WidgetSize node::ScrollViewBase::GetScrollBarSize() const
{
	return { scrolltools::ScrollBar::scrollbar_width, GetSize().h};
}

SDL_FPoint node::ScrollViewBase::GetScrollBarPosition() const
{
	return {GetSize().w - scrolltools::ScrollBar::scrollbar_width, 0};
}

node::WidgetSize node::ScrollViewBase::GetContainedAreaSize() const
{
	return { GetSize().w - scrolltools::ScrollBar::scrollbar_width, GetSize().h };
}

void node::ScrollViewBase::SetScrollInfo(float page_size, float max_position, float position)
{
	m_scrollbar.SetScrollInfo(page_size, max_position);
	m_scrollbar.SetScrollPosition(position);
}

void node::ScrollViewBase::SetScrollPosition(float position)
{
	m_scrollbar.SetScrollPosition(position);
}

node::scrolltools::ScrollBar::ScrollBar(const WidgetSize& size, ScrollViewBase* parent)
	:Widget{size, parent}, m_parent_view{parent}
{
}

void node::scrolltools::ScrollBar::SetScrollInfo(float size, float max)
{
	m_bar_size = size;
	m_bar_extent = size + max;
}

void node::scrolltools::ScrollBar::SetScrollPosition(float position)
{
	m_bar_pos = position;
}

void node::scrolltools::ScrollBar::OnDraw(SDL::Renderer& renderer)
{
	if (m_bar_extent <= m_bar_size)
	{
		return;
	}

	auto rect = GetBarRect();
	const SDL_Color scrollbar_color = renderer.GetColor(ColorRole::scrollbar);
	m_painter.Draw(renderer, rect, 5, scrollbar_color);

}

MI::ClickEvent node::scrolltools::ScrollBar::OnLMBDown(MouseButtonEvent& e)
{
	auto bar_rect = GetBarRect();
	auto point = e.point();
	if (SDL_PointInRectFloat(&point, &bar_rect))
	{
		m_start_drag_top_position = m_bar_pos;
		m_start_drag_mouse_position = e.point().y;
		m_being_dragged = true;
		return MI::ClickEvent::CAPTURE_START;
	}
	return MI::ClickEvent::CLICKED;
}

MI::ClickEvent node::scrolltools::ScrollBar::OnLMBUp(MouseButtonEvent&)
{
	m_being_dragged = false;
	return MI::ClickEvent::CAPTURE_END;
}

void node::scrolltools::ScrollBar::OnMouseMove(MouseHoverEvent& e)
{
	if (m_being_dragged)
	{
		float mouse_move_amount = e.point().y - m_start_drag_mouse_position;
		m_parent_view->RequestPosition(m_start_drag_top_position + mouse_move_amount * m_bar_extent / GetSize().h);
	}
}

SDL_FRect node::scrolltools::ScrollBar::GetBarRect() const
{
	auto rect = GetSize().ToRect();
	rect.y += m_bar_pos * rect.h / m_bar_extent;
	rect.h = m_bar_size / m_bar_extent * GetSize().h;
	return rect;
}
