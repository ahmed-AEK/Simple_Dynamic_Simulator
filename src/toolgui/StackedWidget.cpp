#include "StackedWidget.hpp"
#include "SDL_Framework/SDLRenderer.hpp"
#include "SDL_Framework/SDL_Math.hpp"

#include <algorithm>

node::StackedWidget::StackedWidget(const WidgetSize& size, Widget* parent)
	:Widget{size, parent}
{
}

void node::StackedWidget::Draw(SDL::Renderer& renderer)
{
	if (auto* widget = GetCurrentWidget())
	{
		auto clip = renderer.ClipRect(Widget::WidgetRect(*widget));
		if (clip)
		{
			widget->Draw(renderer);
		}
	}
}

int32_t node::StackedWidget::AddWidget(std::unique_ptr<Widget> widget)
{
	m_widgets.emplace_back(std::move(widget));
	m_widgets.back()->SetParent(this);
	return static_cast<int32_t>(m_widgets.size() - 1);
}

bool node::StackedWidget::SetCurrentIndex(const Widget* ptr)
{
	auto it = std::find_if(m_widgets.begin(), m_widgets.end(), [&](const std::unique_ptr<Widget>& tab) {return tab.get() == ptr; });
	if (it != m_widgets.end())
	{
		return SetCurrentIndex(static_cast<int32_t>(std::distance(m_widgets.begin(), it)));
	}
	return false;
}

bool node::StackedWidget::SetCurrentIndex(int32_t index)
{
	assert(index < WidgetsCount());
	if (!(index < WidgetsCount()) || index < -1)
	{
		return false;
	}

	m_current_widget_index = index;

	Widget* new_tab = index != -1 ? m_widgets[index].get() : nullptr;

	if (new_tab)
	{
		SetFocusProxy(new_tab);
		auto&& size = GetSize();
		new_tab->SetSize(size);
	}
	return true;
}

bool node::StackedWidget::DeleteWidget(int32_t index)
{
	assert(index < WidgetsCount());
	if (!(index < WidgetsCount()) || index < 0)
	{
		return false;
	}
	m_widgets.erase(m_widgets.begin() + index);

	if (WidgetsCount() && m_current_widget_index >= WidgetsCount())
	{
		// closing last tab
		SetCurrentIndex(WidgetsCount() - 1);
	}
	else if (m_current_widget_index > index)
	{
		// closing tab before this one
		SetCurrentIndex(m_current_widget_index - 1);
	}
	else if (m_current_widget_index == index && WidgetsCount())
	{
		// the current widget changed
		SetCurrentIndex(m_current_widget_index);
	}
	else if (WidgetsCount() == 0)
	{
		SetCurrentIndex(-1);
	}
	return true;
}

node::Widget* node::StackedWidget::GetTabWidget(int32_t index)
{
	assert(WidgetsCount() > index);
	if (index == -1)
	{
		return nullptr;
	}
	if (index < WidgetsCount())
	{
		return m_widgets[index].get();
	}
	return nullptr;
}

int32_t node::StackedWidget::GetWidgetIndex(const Widget* widget)
{
	auto it = std::find_if(m_widgets.begin(), m_widgets.end(), 
		[&](const auto& tab) { return tab.get() == widget; });
	if (it != m_widgets.end())
	{
		return static_cast<int32_t>(std::distance(m_widgets.begin(), it));
	}
	return npos;
}

node::Widget* node::StackedWidget::GetCurrentWidget() const
{
	if (WidgetsCount() > m_current_widget_index && m_current_widget_index != -1)
	{
		return m_widgets[m_current_widget_index].get();
	}
	return nullptr;
}

void node::StackedWidget::OnSetSize(const WidgetSize& size)
{
	Widget::OnSetSize(size);
	if (auto* widget = GetCurrentWidget())
	{
		widget->SetSize(GetSize());
	}
}

node::Widget* node::StackedWidget::OnGetInteractableAtPoint(const SDL_FPoint& point)
{
	if (auto* widget = GetCurrentWidget())
	{
		return widget->GetInteractableAtPoint(point - widget->GetPosition());
	}
	return this;
}
