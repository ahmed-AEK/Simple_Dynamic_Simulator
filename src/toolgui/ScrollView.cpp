#include "ScrollView.hpp"
#include "SDL_Framework/SDLRenderer.hpp"

node::ScrollView::ScrollView(const WidgetSize& size, Widget* parent)
	: Widget{size, parent}, m_scrollbar{GetScrollBarSize(), this}
{
}

void node::ScrollView::SetWidget(std::unique_ptr<Widget> widget)
{
	m_contained_widget = std::move(widget);
	if (m_contained_widget)
	{
		m_contained_widget->SetParent(this);
		m_contained_widget->SetPosition({ 0,0 });
		m_contained_widget->SetSize(GetContainedWidgetSize());
		m_scrollbar.SetScrollInfo(GetSize().h, m_contained_widget->GetSize().h);
		m_scrollbar.SetScrollPosition(0);
	}
}



void node::ScrollView::RequestPosition(float new_position)
{
	if (new_position < 0)
	{
		new_position = 0;
	}
	if (auto bar_max_pos = m_scrollbar.GetMaxPosition(); bar_max_pos < new_position)
	{
		m_scrollbar.SetScrollPosition(bar_max_pos);
		m_contained_widget->SetPosition({ 0, -bar_max_pos });
	}
	else
	{
		m_scrollbar.SetScrollPosition(new_position);
		m_contained_widget->SetPosition({ 0, -new_position });
	}
}

bool node::ScrollView::OnScroll(const double amount, const SDL_FPoint&)
{
	if (!m_contained_widget)
	{
		return false;
	}

	auto new_value = m_scrollbar.GetScrollPosition() - amount * 30;
	RequestPosition(static_cast<float>(new_value));
	return true;
}

void node::ScrollView::OnSetSize(const WidgetSize& size)
{
	Widget::OnSetSize(size);
	m_scrollbar.SetPosition(GetScrollBarPosition());
	m_scrollbar.SetSize(GetScrollBarSize());
	if (m_contained_widget)
	{
		m_contained_widget->SetSize(GetContainedWidgetSize());
		m_scrollbar.SetScrollInfo(GetSize().h, m_contained_widget->GetSize().h);
		if (auto bar_max_pos = m_scrollbar.GetMaxPosition(); bar_max_pos < m_scrollbar.GetScrollPosition())
		{
			m_scrollbar.SetScrollPosition(bar_max_pos);
			m_contained_widget->SetPosition({ 0, -bar_max_pos });
		}
	}
}

node::WidgetSize node::ScrollView::GetScrollBarSize() const
{
	return { scrolltools::ScrollBar::scrollbar_width, GetSize().h};
}

SDL_FPoint node::ScrollView::GetScrollBarPosition() const
{
	return {GetSize().w - scrolltools::ScrollBar::scrollbar_width, 0};
}

node::WidgetSize node::ScrollView::GetContainedWidgetSize() const
{
	if (m_contained_widget)
	{
		return { GetSize().w - scrolltools::ScrollBar::scrollbar_width, m_contained_widget->GetSize().h};
	}
	return { GetSize().w - scrolltools::ScrollBar::scrollbar_width, GetSize().h };
}

void node::ScrollView::SetScrollInfo(float size, float max)
{
	m_scrollbar.SetScrollInfo(size, max);
}

node::scrolltools::ScrollBar::ScrollBar(const WidgetSize& size, ScrollView* parent)
	:Widget{size, parent}, m_parent_view{parent}
{
}

void node::scrolltools::ScrollBar::SetScrollInfo(float size, float bar_extent)
{
	m_bar_size = size;
	m_bar_extent = bar_extent;
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
	SDL_Color Black{ 50,50,50,255 };
	m_painter.Draw(renderer, rect, 5, Black);

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
		m_parent_view->RequestPosition(m_start_drag_top_position + mouse_move_amount);
	}
}

SDL_FRect node::scrolltools::ScrollBar::GetBarRect() const
{
	auto rect = GetSize().ToRect();
	rect.y += m_bar_pos * rect.h / m_bar_extent;
	rect.h = m_bar_size / m_bar_extent * GetSize().h;
	return rect;
}
