#include "TabbedView.hpp"
#include "SDL_Framework/SDLRenderer.hpp"

#include <cmath>

node::TabbedView::TabbedView(TTF_Font* font, const WidgetSize& size, Widget* parent)
	:Widget{size, parent}, m_stacked_widget{{0,0}, this}, 
	m_bar{font, {0,static_cast<float>(GetTabsBarHeight())}, this}
{
	SetFocusProxy(&m_stacked_widget);
	m_bar.SetFocusProxy(&m_stacked_widget);
	m_stacked_widget.SetPosition({ 2, static_cast<float>(GetTabsBarHeight()) });
	m_outer_bg_painter.SetDrawSides(false, true, true, true);
	m_inner_bg_painter.SetDrawSides(false, true, true, true);
}

int32_t node::TabbedView::AddTab(std::string tab_name, std::unique_ptr<Widget> widget)
{
	m_stacked_widget.AddWidget(std::move(widget));
	m_tab_names.emplace_back(std::move(tab_name));
	m_bar.AddTab(m_tab_names.back());
	return static_cast<int32_t>(m_tab_names.size() - 1);
}

void node::TabbedView::OnSetSize(const WidgetSize& size)
{
	Widget::OnSetSize(size);

	m_bar.SetSize({ size.w, static_cast<float>(GetTabsBarHeight()) });
	int tab_bar_height = GetTabsBarHeight();	
	m_stacked_widget.SetSize({ size.w - 4, size.h - tab_bar_height - 2 });
}

void node::TabbedView::OnDraw(SDL::Renderer& renderer)
{
	float bar_height = static_cast<float>(GetTabsBarHeight() - 2);
	auto size = GetSize();
	SDL_FRect child_widget_rect{0, bar_height, size.w, size.h - bar_height};
	ThickFilledRoundRect(renderer, child_widget_rect, 8, 2, { 204,204,204,255 }, { 255,255,255,255 }, m_outer_bg_painter, m_inner_bg_painter);
}

int node::TabbedView::GetTabsBarHeight() const
{
	return 30;
}

void node::TabbedView::SetCurrentTabIndex(const Widget* ptr)
{
	SetCurrentTabIndex(m_stacked_widget.GetWidgetIndex(ptr));
}


node::TabBar::TabBar(TTF_Font* font, const WidgetSize& size, TabbedView* parent)
	:Widget{size, parent}, m_font{font}, m_parent{parent}
{
}

void node::TabBar::OnDraw(SDL::Renderer& renderer)
{
	UNUSED_PARAM(renderer);
}

void node::TabBar::OnMouseMove(MouseHoverEvent& e)
{
	if (!m_current_drag_data)
	{
		return;
	}
	auto* dragged_widget = m_current_drag_data->btn_ptr.GetObjectPtr();
	if (!dragged_widget)
	{
		return;
	}

	{
		auto it = std::find(m_buttons_preview_order.begin(), m_buttons_preview_order.end(), dragged_widget);
		if (it == m_buttons_preview_order.end())
		{
			return;
		}

		m_buttons_preview_order.erase(it);
	}
	
	auto mouse_pos = e.point();

	float x_val = 0;
	auto it = m_buttons_preview_order.begin();
	while (it != m_buttons_preview_order.end())
	{
		auto offset = (*it)->GetSize().w;
		if (x_val + offset > mouse_pos.x)
		{
			break;
		}
		x_val += offset;
		it++;
	}
	m_buttons_preview_order.insert(it, dragged_widget);
	ReCalcLayout();
}

MI::ClickEvent node::TabBar::OnLMBDown(MouseButtonEvent& e)
{
	auto click_pos = e.point();
	for (const auto& btn : m_buttons_preview_order)
	{
		SDL_FRect Btn_Rect = btn->WidgetFRect(*btn);
		if (SDL_PointInRectFloat(&click_pos, &Btn_Rect))
		{
			m_current_drag_data = { HandlePtrS<TabButton,Widget>{*btn} };
			return MI::ClickEvent::CAPTURE_START;
		}
	}
	return MI::ClickEvent::NONE;
}
MI::ClickEvent node::TabBar::OnLMBUp(MouseButtonEvent& e)
{
	UNUSED_PARAM(e);
	if (m_current_drag_data)
	{
		m_current_drag_data = std::nullopt;
		return MI::ClickEvent::CAPTURE_END;
	}
	return MI::ClickEvent::NONE;
}

void node::TabBar::AddTab(std::string name)
{
	m_buttons.push_back(
		std::make_unique<TabButton>(m_font, 
			WidgetSize{static_cast<float>(GetTabWidth()), GetSize().h }, this));
	m_buttons_preview_order.push_back(m_buttons.back().get());
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
	auto* btn_addr = (m_buttons.begin() + index)->get();
	m_buttons.erase(m_buttons.begin() + index);
	m_buttons_preview_order.erase(
		std::find(m_buttons_preview_order.begin(), m_buttons_preview_order.end(),btn_addr));
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

void node::TabBar::ButtonClicked(const TabButton* btn)
{
	auto it = std::find_if(m_buttons.begin(), m_buttons.end(), [&](auto&& other) { return other.get() == btn; });
	if (it != m_buttons.end())
	{
		m_parent->SetCurrentTabIndex(static_cast<int32_t>( std::distance(m_buttons.begin(), it) ));
	}
}

void node::TabBar::ButtonXClicked(const TabButton* btn)
{
	auto it = std::find_if(m_buttons.begin(), m_buttons.end(), [&](auto&& other) { return other.get() == btn; });
	if (it != m_buttons.end())
	{
		m_parent->RequestDeleteTab(static_cast<int32_t>( std::distance(m_buttons.begin(), it) ));
	}
}

void node::TabBar::SetTabName(int32_t idx, std::string_view name)
{
	assert(idx != npos);
	if (idx == npos)
	{
		return;
	}

	m_buttons[idx]->SetText(std::string{ name });
}

void node::TabBar::ReCalcLayout()
{
	float x_val = 0;
	float y_val = 0;
	for (auto&& btn : m_buttons_preview_order)
	{
		btn->SetPosition({ x_val, y_val });
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

int32_t node::TabbedView::GetWidgetIndex(const Widget* widget)
{
	return m_stacked_widget.GetWidgetIndex(widget);
}

void node::TabbedView::SetTabName(int32_t idx, std::string_view name)
{
	assert(idx != npos);
	if (idx == npos)
	{
		return;
	}

	m_tab_names[idx] = std::string{ name };
	m_bar.SetTabName(idx, name);
}

node::TabButton::TabButton(TTF_Font* font, const WidgetSize& size, TabBar* parent)
	:Widget{size, parent}, m_tab_text{font}, m_X_painter{font}, m_parent{parent}
{
	m_X_painter.SetText("X");
	m_outer_painter.SetDrawSides(true, true, false, false);
	m_inner_painter.SetDrawSides(true, true, false, false);
}

void node::TabButton::OnDraw(SDL::Renderer& renderer)
{
	auto rect = GetSize().ToRect();

	SDL_Color background_color{ 255,255,255,255 };
	if (!IsActiveBtn())
	{
		if (m_mouse_hovered)
		{
			background_color = SDL_Color{ 240,240,240,255 };
		}
		else
		{
			background_color = SDL_Color{ 204, 204, 204, 255};
		}
	}


	SDL_Color outlineColor{ 204,204,204,255 };
	if (IsActiveBtn())
	{
		auto new_rect = rect;
		new_rect.h += 2;
		ThickFilledRoundRect(renderer, new_rect, 8, 2, outlineColor, background_color, m_outer_painter, m_inner_painter);
	}
	else
	{
		if (m_mouse_hovered)
		{
			auto new_rect = rect;
			new_rect.h += 2;
			ThickFilledRoundRect(renderer, new_rect, 8, 2, outlineColor, background_color, m_outer_painter, m_inner_painter);
		}
		else
		{
			m_outer_painter.Draw(renderer, rect, 8, background_color);
		}
	}

	SDL_Color Black{ 50,50,50,255 };
	m_tab_text.Draw(renderer, { rect.x + 6, rect.y + 6}, Black);

	if (m_mouse_hovered)
	{
		auto X_Rect = GetXBtnRect();
		ThickFilledRoundRect(renderer, X_Rect, 6, 1, { 180,180,180,255 }, background_color, m_X_btn_painter_outer, m_X_btn_painter_inner);
		SDL_FRect X_Rect_inner = m_X_painter.GetRect(renderer, Black);
		m_X_painter.Draw(renderer, { X_Rect.x + X_Rect.w / 2 - X_Rect_inner.w / 2 , X_Rect.y + X_Rect.h / 2 - X_Rect_inner.h / 2 }, Black);
	}
}

void node::TabButton::SetText(std::string name)
{
	int measured_width = 0;
	size_t measured_length = 0;
	auto success = TTF_MeasureString(m_tab_text.GetFont(), name.c_str(), name.size(), 
		static_cast<int>(GetXBtnStart() - 6), &measured_width, &measured_length);
	if (!success)
	{
		logger(logging::LogCategory::GUI).LogError("Failed to size string in tab!");
		m_tab_text.SetText("Error!");
		return;
	}
	if (measured_length != name.size())
	{
		name = name.substr(0, measured_length) + "....";
	}
	m_tab_text.SetText(std::move(name));
}

void node::TabButton::SetActive(bool value)
{
	m_active = value;
}

void node::TabButton::OnMouseOut(MouseHoverEvent&)
{
	m_mouse_hovered = false;
	m_exit_initiated = false;
}

void node::TabButton::OnMouseIn(MouseHoverEvent&)
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
	return MI::ClickEvent::NONE;
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
	auto rect = GetSize().ToRect();
	rect.x = GetXBtnStart();
	rect.w = 25;
	rect.y += 4;
	rect.h -= 6;
	return rect;
}

float node::TabButton::GetXBtnStart() const
{
	return GetSize().w - 30;
}
