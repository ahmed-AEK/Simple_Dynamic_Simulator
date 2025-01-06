#include "TabbedView.hpp"

#include <cmath>

node::TabbedView::TabbedView(TTF_Font* font, const SDL_FRect& rect, Widget* parent)
	:Widget{rect, parent}, m_stacked_widget{{0,0,0,0}, this}, m_bar{font, {0,0,0,0}, this}
{
	SetFocusProxy(&m_stacked_widget);
	m_bar.SetFocusProxy(&m_stacked_widget);
}

int32_t node::TabbedView::AddTab(std::string tab_name, std::unique_ptr<Widget> widget)
{
	m_stacked_widget.AddWidget(std::move(widget));
	m_tab_names.emplace_back(std::move(tab_name));
	m_bar.AddTab(m_tab_names.back());
	return static_cast<int32_t>(m_tab_names.size() - 1);
}

void node::TabbedView::OnSetRect(const SDL_FRect& rect)
{
	Widget::OnSetRect(rect);

	m_bar.SetRect({ rect.x, rect.y, rect.w, static_cast<float>(GetTabsBarHeight()) });
	int tab_bar_height = GetTabsBarHeight();
	SDL_FRect middle_rect{ rect.x, rect.y + tab_bar_height, rect.w, rect.h - tab_bar_height };
	m_stacked_widget.SetRect(middle_rect);
}

node::Widget* node::TabbedView::OnGetInteractableAtPoint(const SDL_FPoint& point)
{
	if (auto ptr = m_bar.GetInteractableAtPoint(point))
	{
		return ptr;
	}
	if (auto* ptr = m_stacked_widget.GetInteractableAtPoint(point))
	{
		return ptr;
	}
	return this;
}

int node::TabbedView::GetTabsBarHeight() const
{
	return 30;
}

void node::TabbedView::Draw(SDL_Renderer* renderer)
{
	m_bar.Draw(renderer);
	m_stacked_widget.Draw(renderer);
}

void node::TabbedView::SetCurrentTabIndex(Widget* ptr)
{
	SetCurrentTabIndex(m_stacked_widget.GetWidgetIndex(ptr));
}


node::TabBar::TabBar(TTF_Font* font, const SDL_FRect& rect, TabbedView* parent)
	:Widget{rect, parent}, m_font{font}, m_parent{parent}
{
}

void node::TabBar::Draw(SDL_Renderer* renderer)
{
	
	SDL_FRect rect = GetRect();
	SDL_Color outlineColor{ 50,50,50,255 };
	SDL_SetRenderDrawColor(renderer, outlineColor.r, outlineColor.g, outlineColor.b, 255);
	SDL_FRect lower_rect{ rect.x, rect.y + rect.h - 2, rect.w, 2 };
	SDL_RenderFillRect(renderer, &lower_rect);

	for (auto&& btn : m_buttons)
	{
		btn->Draw(renderer);
	}

}

void node::TabBar::AddTab(std::string name)
{
	m_buttons.push_back(std::make_unique<TabButton>(m_font, SDL_FRect{0,0, static_cast<float>(GetTabWidth()), GetRect().h - 4}, this));
	m_buttons.back()->SetText(std::move(name));
	m_buttons.back()->SetFocusProxy(this);
	ReCalcLayout();
}

void node::TabBar::DeleteTab(int32_t index)
{
	assert(index < TabsCount());
	if (!(index < TabsCount()))
	{
		return;
	}
	m_buttons.erase(m_buttons.begin() + index);
	ReCalcLayout();
}

void node::TabBar::SetActiveTabIndex(int32_t index)
{
	assert(index < TabsCount());
	int32_t old_active_index = m_active_tab;
	m_active_tab = index;
	if (old_active_index < TabsCount() && old_active_index != -1)
	{
		m_buttons[old_active_index]->SetActive(false);
	}
	if (index < TabsCount() && index != -1)
	{
		m_buttons[index]->SetActive(true);
	}
}

void node::TabBar::ButtonClicked(TabButton* btn)
{
	auto it = std::find_if(m_buttons.begin(), m_buttons.end(), [&](auto&& other) { return other.get() == btn; });
	if (it != m_buttons.end())
	{
		m_parent->SetCurrentTabIndex(static_cast<int32_t>( std::distance(m_buttons.begin(), it) ));
	}
}

void node::TabBar::ButtonXClicked(TabButton* btn)
{
	auto it = std::find_if(m_buttons.begin(), m_buttons.end(), [&](auto&& other) { return other.get() == btn; });
	if (it != m_buttons.end())
	{
		m_parent->RequestDeleteTab(static_cast<int32_t>( std::distance(m_buttons.begin(), it) ));
	}
}

void node::TabBar::OnSetRect(const SDL_FRect& rect)
{
	Widget::OnSetRect(rect);
	ReCalcLayout();
}

node::Widget* node::TabBar::OnGetInteractableAtPoint(const SDL_FPoint& point)
{
	for (auto&& btn : m_buttons)
	{
		if (auto ptr = btn->GetInteractableAtPoint(point))
		{
			return ptr;
		}
	}
	return nullptr;
}

void node::TabBar::ReCalcLayout()
{
	float x_val = GetRect().x + 2;
	float y_val = GetRect().y;
	float height = GetRect().h - 2;
	for (auto&& btn : m_buttons)
	{
		btn->SetRect(SDL_FRect(x_val, y_val, static_cast<float>(GetTabWidth()), height));
		x_val += GetTabWidth() + 2;
	}
}

void node::TabbedView::SetCurrentTabIndex(int32_t index)
{
	auto old_idx = m_stacked_widget.GetCurrentIndex();

	if (!m_stacked_widget.SetCurrentIndex(index))
	{
		return;
	}
	
	m_bar.SetActiveTabIndex(index);

	auto* widget = m_stacked_widget.GetCurrentWidget();

	Notify(TabsChangeEvent{ TabIndexChangeEvent{old_idx, index, widget} });
}

void node::TabbedView::RequestDeleteTab(int32_t index)
{
	assert(index < TabsCount());
	if (!(index < TabsCount()))
	{
		return;
	}

	Notify(TabsChangeEvent{ TabCloseRequestEvent{index, m_stacked_widget.GetCurrentWidget()}});
}

void node::TabbedView::DeleteTab(int32_t index)
{	
	auto old_idx = m_stacked_widget.GetCurrentIndex();
	if (!m_stacked_widget.DeleteWidget(index))
	{
		return;
	}

	auto new_idx = m_stacked_widget.GetCurrentIndex();
	m_bar.DeleteTab(index);
	m_bar.SetActiveTabIndex(new_idx);

	m_tab_names.erase(m_tab_names.begin() + index);

	Notify(TabsChangeEvent{ TabIndexChangeEvent{old_idx, new_idx, m_stacked_widget.GetCurrentWidget()} });
}

node::Widget* node::TabbedView::GetTabWidget(int32_t index)
{
	return m_stacked_widget.GetTabWidget(index);
}

int32_t node::TabbedView::GetWidgetIndex(Widget* widget)
{
	return m_stacked_widget.GetWidgetIndex(widget);
}

node::TabButton::TabButton(TTF_Font* font, const SDL_FRect& rect, TabBar* parent)
	:Widget{rect, parent}, m_tab_text{font}, m_X_painter{font}, m_parent{parent}
{
	m_X_painter.SetText("X");
}

void node::TabButton::Draw(SDL_Renderer* renderer)
{
	auto rect = GetRect();

	SDL_Color background_color{ 255,255,255,255 };
	if (!GetActive())
	{
		if (m_mouse_hovered)
		{
			background_color = SDL_Color{ 240,240,240,255 };
		}
		else
		{
			background_color = SDL_Color{ 210,210,210,255 };
		}
	}


	SDL_Color outlineColor{ 50,50,50,255 };
	if (GetActive())
	{
		m_outer_painter.Draw(renderer, rect, 8, outlineColor);
		SDL_FRect lower_rect{ rect.x, std::floor(rect.y + rect.h / 2), rect.w,std::ceil(rect.h / 2) };
		SDL_SetRenderDrawColor(renderer, outlineColor.r, outlineColor.g, outlineColor.b, outlineColor.a);
		SDL_RenderFillRect(renderer, &lower_rect);

		SDL_FRect inner_rect{ rect };
		inner_rect.x += 2;
		inner_rect.w -= 4;
		inner_rect.h -= 2;
		inner_rect.y += 2;
		m_inner_painter.Draw(renderer, inner_rect, 6, background_color);
		lower_rect.x += 2;
		lower_rect.w -= 4;
		SDL_SetRenderDrawColor(renderer, 
			background_color.r, background_color.g, background_color.b, background_color.a);
		SDL_RenderFillRect(renderer, &lower_rect);
	}
	else
	{
		m_outer_painter.Draw(renderer, rect, 8, background_color);
		SDL_FRect lower_rect{ rect.x, std::floor(rect.y + rect.h / 2), rect.w,std::ceil(rect.h / 2) };
		SDL_SetRenderDrawColor(renderer,
			background_color.r, background_color.g, background_color.b, background_color.a);
		SDL_RenderFillRect(renderer, &lower_rect);
	}

	SDL_Color Black{ 50,50,50,255 };
	m_tab_text.Draw(renderer, { rect.x + 4, rect.y + 2}, Black);

	if (m_mouse_hovered)
	{
		auto X_Rect = GetXBtnRect();
		ThickFilledRoundRect(renderer, X_Rect, 6, 1, { 180,180,180,255 }, background_color, m_outer_painter, m_inner_painter);
		SDL_FRect X_Rect_inner = m_X_painter.GetRect(renderer, Black);
		m_X_painter.Draw(renderer, { X_Rect.x + X_Rect.w / 2 - X_Rect_inner.w / 2 , X_Rect.y + X_Rect.h / 2 - X_Rect_inner.h / 2 }, Black);
	}
}

void node::TabButton::SetText(std::string name)
{
	m_tab_text.SetText(std::move(name));
}

void node::TabButton::SetActive(bool value)
{
	m_active = value;
}

void node::TabButton::OnMouseOut()
{
	m_mouse_hovered = false;
	m_exit_initiated = false;
}

void node::TabButton::OnMouseIn()
{
	m_mouse_hovered = true;
	m_exit_initiated = false;
}

void node::TabButton::OnMouseMove(MouseHoverEvent& e)
{
	if (m_exit_initiated)
	{
		auto X_rect = GetXBtnRect();
		auto point = e.point();
		if (!SDL_PointInRectFloat(&point, &X_rect))
		{
			m_exit_initiated = false;
		}
	}
}

MI::ClickEvent node::TabButton::OnLMBDown(MouseButtonEvent& e)
{
	auto X_rect = GetXBtnRect();
	auto point = e.point();
	if (SDL_PointInRectFloat(&point, &X_rect))
	{
		m_exit_initiated = true;
		return MI::ClickEvent::CLICKED;
	}
	
	m_parent->ButtonClicked(this);
	return MI::ClickEvent::CLICKED;
}

MI::ClickEvent node::TabButton::OnLMBUp(MouseButtonEvent& e)
{
	if (m_exit_initiated)
	{
		m_parent->ButtonXClicked(this);
		return MI::ClickEvent::CLICKED;
	}
	return Widget::OnLMBUp(e);
}

SDL_FRect node::TabButton::GetXBtnRect() const
{
	auto rect = GetRect();
	rect.x = rect.x + rect.w - 30;
	rect.w = 25;
	rect.y += 4;
	rect.h -= 6;
	return rect;
}
