#include "SidePanel.hpp"
#include "toolgui/Scene.hpp"
#include "SDL_Framework/Utility.hpp"
#include "SDL_Framework/SDL_Math.hpp"
#include "toolgui/Application.hpp"
#include <cmath>

static double easeOut(double val) { return 2 * val - val * val; }

node::SidePanel::SidePanel(PanelSide side, const WidgetSize& size, Widget* parent)
	: Widget(size, parent),
	m_knob_drawer_left{"assets/arrow_left.svg", 30,knob_height*3/5}, 
	m_knob_drawer_right{"assets/arrow_right.svg", 30, knob_height * 3 / 5 }, m_side(side)
{
	RepositionWidget();
}

void node::SidePanel::Expand() noexcept
{
	if (m_state == PanelState::open)
	{
		return;
	}
	m_last_action_time = SDL_GetTicks();
	m_state = PanelState::openning;
	m_updateTaskId = GetApp()->AddUpdateTask(UpdateTask::FromWidget(*this, [this]() { this->UpdatePanelMotion(); } ));
}

void node::SidePanel::Retract() noexcept
{
	if (m_state == PanelState::closed)
	{
		return;
	}
	m_last_action_time = SDL_GetTicks();
	m_state = PanelState::closing;
	m_updateTaskId = GetApp()->AddUpdateTask(UpdateTask::FromWidget(*this, [this]() { this->UpdatePanelMotion(); } ));
}

void node::SidePanel::SetWidget(std::unique_ptr<Widget> widget)
{
	m_contained_widget = std::move(widget);
	if (m_contained_widget)
	{
		auto widget_rect = CalculateChildWidgetRect();
		m_contained_widget->SetPosition({ widget_rect.x, widget_rect.y });
		m_contained_widget->SetSize({widget_rect.w, widget_rect.h});
	}
}

void node::SidePanel::OnDraw(SDL::Renderer& renderer)
{
	SDL_FRect draw_area = GetSize().ToRect();
	draw_area.x += knob_width;
	draw_area.w -= knob_width;
	// draw main widget part
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &draw_area);
	SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
	SDL_FRect inner_rect{ draw_area.x + 2, draw_area.y + 2, draw_area.w - 4, draw_area.h - 4 };
	SDL_RenderFillRect(renderer, &inner_rect);

	// draw side knob
	DrawKnob(renderer);
}

MI::ClickEvent node::SidePanel::OnLMBDown(MouseButtonEvent& e)
{
	SDL_FPoint current_mouse_point{ e.point() };
	SDL_FRect knobRect = GetKnobRect();
	if (!SDL_PointInRectFloat(&current_mouse_point, &knobRect)) { return MI::ClickEvent::NONE; }
	switch (m_state)
	{
	case PanelState::closed:
		Expand();
		return MI::ClickEvent::CLICKED;
		break;
	case PanelState::open:
		Retract();
		return MI::ClickEvent::CLICKED;
		break;
	default:
		break;
	}
	return MI::ClickEvent::NONE;
}

void node::SidePanel::OnSetSize(const WidgetSize& size)
{
	Widget::OnSetSize(size);
	auto widget_rect = CalculateChildWidgetRect();
	m_contained_widget->SetSize({ widget_rect.w, widget_rect.h });
}

void node::SidePanel::UpdateWindowSize(const WidgetSize& size)
{
	if (PanelSide::right == m_side)
	{
		m_corner_position = size.w;
	}
	else
	{
		m_corner_position = 0;
	}
	SetPosition({ GetPosition().x, 0 });
	SetSize({ GetSize().w, size.h});
	RepositionWidget();
}

node::Widget* node::SidePanel::OnGetInteractableAtPoint(const SDL_FPoint& point)
{
	auto&& knobRect = GetKnobRect();
	if (SDL_PointInRectFloat(&point, &knobRect))
	{
		return this;
	}

	
	if (m_contained_widget)
	{
		if (auto* result = m_contained_widget->GetInteractableAtPoint(point - m_contained_widget->GetPosition()))
		{
			return result;
		}
	}

	
	SDL_FRect inner_rect = GetSize().ToRect();
	switch (m_side)
	{
	case PanelSide::right:
		inner_rect.x += knob_width;
		break;
	case PanelSide::left:
		break;
	}
	inner_rect.w -= knob_width;
	if (SDL_PointInRectFloat(&point, &inner_rect))
	{
		return this;
	}

	return nullptr;
}

void node::SidePanel::UpdatePanelMotion()
{
	if (PanelState::openning == m_state)
	{
		auto passed_ticks = SDL_GetTicks() - m_last_action_time;
		auto passed_time = passed_ticks / TICKS_PER_SECOND;
		m_expand_percent = passed_time / TRANSITION_TIME;
		if (m_expand_percent >= 1)
		{
			m_expand_percent = 1;
			m_state = PanelState::open;
			GetApp()->RemoveUpdateTask(m_updateTaskId);
			m_updateTaskId = 0;
		}
		RepositionWidget();
	}
	if (PanelState::closing == m_state)
	{
		auto passed_ticks = SDL_GetTicks() - m_last_action_time;
		auto passed_time = passed_ticks / TICKS_PER_SECOND;
		m_expand_percent = 1 - passed_time / TRANSITION_TIME;
		if (m_expand_percent <= 0)
		{
			m_expand_percent = 0;
			m_state = PanelState::closed;
			GetApp()->RemoveUpdateTask(m_updateTaskId);
			m_updateTaskId = 0;
		}
		RepositionWidget();
	}
}

SDL_FRect node::SidePanel::CalculateChildWidgetRect()
{
	switch (m_side)
	{
	case PanelSide::right:
		return { knob_width + widget_margin, widget_margin, GetSize().w - knob_width - 2 * widget_margin, GetSize().h - 2 * widget_margin };
		break;
	case PanelSide::left:
		assert(false);
		break;
	}
	return {};
}

SDL_FRect node::SidePanel::GetKnobRect()
{
	switch (m_side)
	{
	case PanelSide::right:
		return { 0, std::floor(GetSize().h / 2 - knob_height / 2), knob_width, knob_height };
	case PanelSide::left:
		assert(false);
		return {};
	}
	return {};
}

void node::SidePanel::DrawKnob(SDL_Renderer* renderer)
{
	SDL_FRect knob_rect = GetKnobRect();
	SDL_Color outerColor{ 0,0,0,255 };
	SDL_Color innerColor{ 220,220,220,255 };
	ThickFilledRoundRect(renderer, knob_rect, 15, 2, outerColor, innerColor,
		m_outer_painter, m_inner_painter);

	// redraw right part to remove the roundrect there
	SDL_FRect right_rect{ std::floor(knob_rect.x + knob_rect.w / 2), knob_rect.y, std::ceil(knob_rect.w / 2), knob_rect.h };
	SDL_SetRenderDrawColor(renderer, outerColor.r, outerColor.g, outerColor.b, 255);
	SDL_RenderFillRect(renderer, &right_rect);
	right_rect.y += 2;
	right_rect.h -= 4;
	SDL_SetRenderDrawColor(renderer, innerColor.r, innerColor.g, innerColor.b, 255);
	SDL_RenderFillRect(renderer, &right_rect);

	// draw arrow
	SVGRasterizer* knob_rasterizer = nullptr;

	switch (m_side)
	{
	case PanelSide::right:

	{
		switch (m_state)
		{
		case PanelState::closed:
		case PanelState::openning:
		{
			knob_rasterizer = &m_knob_drawer_left;
			break;
		}
		case PanelState::open:
		case PanelState::closing:
		{
			knob_rasterizer = &m_knob_drawer_right;
			break;
		}
		}
	}
	break;

	case PanelSide::left:
		assert(0);
		break;
	}
	knob_rasterizer->Draw(renderer, knob_rect.x + 5, knob_rect.y + knob_height * 1 / 5);
}

void node::SidePanel::RepositionWidget()
{
	switch (m_side)
	{
	case PanelSide::right:
	{
		auto width = GetSize().w - knob_width;
		SetPosition({ static_cast<float>(m_corner_position - width * easeOut(m_expand_percent) - knob_width),
			0.0f });
		break;
	}
	case PanelSide::left:
		assert(false);
		break;
	}
}

