#include "SidePanel.hpp"
#include "toolgui/Scene.hpp"
#include "SDL_Framework/Utility.hpp"
#include "SDL_Framework/SDL_Math.hpp"
#include "toolgui/Application.hpp"
#include <cmath>


node::SidePanel::SidePanel(PanelSide side, TTF_Font* font, const WidgetSize& size, Widget* parent)
	: Widget(size, parent), 
	m_close_btn{ WidgetSize{30,35}, "Close Panel", "assets/arrow_right.svg",
	[this]() {this->OnRequestClosePanel(); }, this }, 
	m_title_painter{ font }, m_side(side)
{
	m_close_btn.SetPosition(GetCloseBtnPosition());
}

void node::SidePanel::SetWidget(std::unique_ptr<Widget> widget)
{
	m_contained_widget = std::move(widget);
	if (m_contained_widget)
	{
		auto widget_rect = CalculateChildWidgetRect();
		m_contained_widget->SetParent(this);
		m_contained_widget->SetPosition({ widget_rect.x, widget_rect.y });
		m_contained_widget->SetSize({widget_rect.w, widget_rect.h});
	}
}

void node::SidePanel::SetTitle(std::string title)
{
	m_title_painter.SetText(std::move(title));
}

void node::SidePanel::OnDraw(SDL::Renderer& renderer)
{
	SDL_FRect draw_area = GetSize().ToRect();
	// draw main widget part
	ThickFilledRoundRect(renderer, draw_area, 8, 2, SDL_Color{ 204,204,204,255 }, SDL_Color{ 255, 255, 255, 255 }, m_outer_painter, m_inner_painter);
	
	SDL_Color Black{ 50,50,50,255 };
	m_title_painter.Draw(renderer, { widget_margin + 5, widget_margin + 5 }, Black);
}

MI::ClickEvent node::SidePanel::OnLMBDown(MouseButtonEvent&)
{
	return MI::ClickEvent::NONE;
}

void node::SidePanel::OnSetSize(const WidgetSize& size)
{
	Widget::OnSetSize(size);
	auto widget_rect = CalculateChildWidgetRect();
	m_contained_widget->SetSize({ widget_rect.w, widget_rect.h });
}

SDL_FRect node::SidePanel::CalculateChildWidgetRect()
{
	return { widget_margin, TitleHeight + widget_margin, GetSize().w - 2 * widget_margin, GetSize().h - 2 * widget_margin - TitleHeight };
}

void node::SidePanel::OnRequestClosePanel()
{
	Notify(PanelCloseRequest{ this });
}

SDL_FPoint node::SidePanel::GetCloseBtnPosition()
{
	return SDL_FPoint{GetSize().w - m_close_btn.GetSize().w - widget_margin, widget_margin};
}

