#include "TabbedView.hpp"

node::TabbedView::TabbedView(TTF_Font* font, const SDL_FRect& rect, Widget* parent)
	:Widget{rect, parent}, m_bar{font, {0,0,0,0},this}
{
}

size_t node::TabbedView::AddTab(std::string tab_name, std::unique_ptr<Widget> widget)
{
	m_tabs.emplace_back(std::move(tab_name), std::move(widget));
	m_tabs.back().widget->SetParent(this);
	m_bar.AddTab(m_tabs.back().name);
	return m_tabs.size() - 1;
}

void node::TabbedView::OnSetRect(const SDL_FRect& rect)
{
	Widget::OnSetRect(rect);

	m_bar.SetRect({ rect.x, rect.y, rect.w, static_cast<float>(GetTabsBarHeight()) });
	if (m_tabs.size() > m_current_tab_index)
	{
		int tab_bar_height = GetTabsBarHeight();
		SDL_FRect new_rect{ rect.x, rect.y + tab_bar_height, rect.w, rect.h - tab_bar_height };
		if (new_rect.h < 0)
		{
			new_rect.h = 0;
		}
		m_tabs[m_current_tab_index].widget->SetRect(new_rect);
	}
}

node::Widget* node::TabbedView::OnGetInteractableAtPoint(const SDL_FPoint& point)
{
	if (auto ptr = m_bar.GetInteractableAtPoint(point))
	{
		return ptr;
	}

	if (m_tabs.size() > m_current_tab_index)
	{
		if (auto ptr = m_tabs[m_current_tab_index].widget->GetInteractableAtPoint(point))
		{
			return ptr;
		}
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
	if (m_tabs.size() > m_current_tab_index)
	{
		m_tabs[m_current_tab_index].widget->Draw(renderer);
	}
}

void node::TabbedView::SetCurrentTabIndex(Widget* ptr)
{
	auto it = std::find_if(m_tabs.begin(), m_tabs.end(), [&](const TabData& tab) {return tab.widget.get() == ptr; });
	if (it != m_tabs.end())
	{
		SetCurrentTabIndex(std::distance(m_tabs.begin(), it));
	}
}


node::TabBar::TabBar(TTF_Font* font, const SDL_FRect& rect, TabbedView* parent)
	:Widget{rect, parent}, m_font{font}, m_parent{parent}
{
}

void node::TabBar::Draw(SDL_Renderer* renderer)
{
	SDL_FRect rect = GetRect();
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &rect);
	rect.x += 2;
	rect.w -= 4;
	rect.h -= 2;
	SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
	SDL_RenderFillRect(renderer, &rect);

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

void node::TabBar::DeleteTab(size_t index)
{
	assert(index < m_buttons.size());
	if (!(index < m_buttons.size()))
	{
		return;
	}
	m_buttons.erase(m_buttons.begin() + index);
	ReCalcLayout();
}

void node::TabBar::SetActiveTabIndex(size_t index)
{
	assert(index < m_buttons.size());
	size_t old_active_index = m_active_tab;
	m_active_tab = index;
	if (old_active_index < m_buttons.size())
	{
		m_buttons[old_active_index]->SetActive(false);
	}
	if (index < m_buttons.size())
	{
		m_buttons[index]->SetActive(true);
	}
}

void node::TabBar::ButtonClicked(TabButton* btn)
{
	auto it = std::find_if(m_buttons.begin(), m_buttons.end(), [&](auto&& other) { return other.get() == btn; });
	if (it != m_buttons.end())
	{
		m_parent->SetCurrentTabIndex(std::distance(m_buttons.begin(), it));
	}
}

void node::TabBar::ButtonXClicked(TabButton* btn)
{
	auto it = std::find_if(m_buttons.begin(), m_buttons.end(), [&](auto&& other) { return other.get() == btn; });
	if (it != m_buttons.end())
	{
		m_parent->RequestDeleteTab(std::distance(m_buttons.begin(), it));
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
		x_val += GetTabWidth();
	}
}

void node::TabbedView::SetCurrentTabIndex(size_t index)
{
	assert(index < m_tabs.size());
	if (!(index < m_tabs.size()))
	{
		return;
	}
	auto old_idx = m_current_tab_index;

	m_current_tab_index = index;
	m_bar.SetActiveTabIndex(index);
	SetFocusProxy(m_tabs[m_current_tab_index].widget.get());
	m_bar.SetFocusProxy(m_tabs[m_current_tab_index].widget.get());

	auto&& rect = GetRect();
	SDL_FRect new_rect{ rect.x, rect.y + GetTabsBarHeight(), rect.w, rect.h - GetTabsBarHeight()};
	m_tabs[m_current_tab_index].widget->SetRect(new_rect);
	Notify(TabsChangeEvent{ TabIndexChangeEvent{old_idx, index, m_tabs[m_current_tab_index].widget.get()} });
}

void node::TabbedView::RequestDeleteTab(size_t index)
{
	assert(index < m_tabs.size());
	if (!(index < m_tabs.size()))
	{
		return;
	}

	Notify(TabsChangeEvent{ TabCloseRequestEvent{index, m_tabs[index].widget.get()} });
}

void node::TabbedView::DeleteTab(size_t index)
{
	assert(index < m_tabs.size());
	if (!(index < m_tabs.size()))
	{
		return;
	}
	m_tabs.erase(m_tabs.begin() + index);
	m_bar.DeleteTab(index);
	if (m_tabs.size() && m_current_tab_index >= m_tabs.size())
	{
		SetCurrentTabIndex(m_tabs.size() - 1);
	}
	else if (m_current_tab_index > index)
	{
		SetCurrentTabIndex(m_current_tab_index - 1);
	}
	else if (m_current_tab_index == index)
	{
		// the current widget changed
		SetCurrentTabIndex(m_current_tab_index);
	}
}

node::Widget* node::TabbedView::GetTabWidget(size_t index)
{
	assert(m_tabs.size() > index);
	return m_tabs[index].widget.get();
}

std::optional<size_t> node::TabbedView::GetWidgetIndex(Widget* widget)
{
	auto it = std::find_if(m_tabs.begin(), m_tabs.end(), [&](const auto& tab) { return tab.widget.get() == widget; });
	if (it != m_tabs.end())
	{
		return std::distance(m_tabs.begin(), it);
	}
	return std::nullopt;
}
node::TabButton::TabButton(TTF_Font* font, const SDL_FRect& rect, TabBar* parent)
	:Widget{rect, parent}, m_tab_text{font}, m_X_painter{font}, m_parent{parent}
{
	m_X_painter.SetText("X");
}

void node::TabButton::Draw(SDL_Renderer* renderer)
{
	auto rect = GetRect();

	if (GetActive())
	{
		rect.h += 2;
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderFillRect(renderer, &rect);
		SDL_FRect btn_bar_rect{ rect.x, rect.y + rect.h - 2, rect.w, 2 };
		SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
		SDL_RenderFillRect(renderer, &btn_bar_rect);
	}
	else if (m_mouse_hovered)
	{
		SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
		SDL_RenderFillRect(renderer, &rect);
	}

	SDL_Color Black{ 50,50,50,255 };
	m_tab_text.Draw(renderer, { rect.x + 2, rect.y }, Black);
	SDL_FRect sep_rect{ rect.x + rect.w - 2, rect.y, 2, rect.h };
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &sep_rect);

	if (m_mouse_hovered)
	{
		auto X_Rect = GetXBtnRect();
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderRect(renderer, &X_Rect);
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
	rect.y += 2;
	rect.h -= 4;
	return rect;
}
