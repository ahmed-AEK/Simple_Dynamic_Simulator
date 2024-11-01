#include "SidePanel.hpp"
#include "toolgui/Scene.hpp"
#include "SDL_Framework/Utility.hpp"
#include "toolgui/Application.hpp"

static double easeOut(double val) { return 2 * val - val * val; }

node::SidePanel::SidePanel(PanelSide side, const SDL_FRect& rect, Widget* parent)
	: Widget(rect, parent), m_side(side)
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
		m_contained_widget->SetRect(CalculateChildWidgetRect());
	}
}

void node::SidePanel::Draw(SDL_Renderer* renderer)
{
	SDL_FRect draw_area = GetRect();
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

	// draw inner widget
	if (m_state != PanelState::closed && m_contained_widget)
	{
		m_contained_widget->Draw(renderer);
	}

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

void node::SidePanel::UpdateWindowSize(const SDL_FRect& rect)
{
	if (PanelSide::right == m_side)
	{
		m_corner_position = rect.x + rect.w;
	}
	else
	{
		m_corner_position = rect.x;
	}
	SetRect({ GetRect().x, rect.y, GetRect().w, rect.h });
	RepositionWidget();
}

node::Widget* node::SidePanel::OnGetInteractableAtPoint(const SDL_FPoint& point)
{
	auto&& knobRect = GetKnobRect();
	if (SDL_PointInRectFloat(&point, &knobRect))
	{
		return this;
	}
	if (m_contained_widget &&
		SDL_PointInRectFloat(&point, &m_contained_widget->GetRect()))
	{
		return m_contained_widget.get();
	}
	else
	{
		SDL_FRect inner_rect = GetRect();
		switch (m_side)
		{
		case PanelSide::right:
			inner_rect.x += knob_width;
			break;
		case PanelSide::left:
			inner_rect.x -= knob_width;
			break;
		}
		inner_rect.w -= knob_width;
		if (SDL_PointInRectFloat(&point, &inner_rect))
		{
			return this;
		}

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
		return { GetRect().x + knob_width + widget_margin, GetRect().y + widget_margin, GetRect().w - knob_width - 2 * widget_margin, GetRect().h - 2 * widget_margin };
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
		return { GetRect().x, std::floor(GetRect().y + GetRect().h / 2 - knob_height / 2), knob_width, knob_height };
	case PanelSide::left:
		assert(false);
		return {};
	}
	return {};
}

void node::SidePanel::DrawKnob(SDL_Renderer* renderer)
{
	SDL_FRect knob_rect = GetKnobRect();
	ThickFilledRoundRect(renderer, knob_rect, 15, 2, SDL_Color{ 0,0,0,255 }, SDL_Color{ 220,220,220,255 },
		m_outer_painter, m_inner_painter);
	// draw arrow
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	std::array<SDL_Vertex, 3> arrow_verts;

	switch (m_side)
	{
	case PanelSide::right:

	{
		switch (m_state)
		{
		case PanelState::closed:
		case PanelState::openning:
		{
			arrow_verts =
				std::array<SDL_Vertex, 3>{
				SDL_Vertex{ static_cast<float>(knob_rect.x + 5), static_cast<float>(knob_rect.y + knob_height / 2)
				, {0,0,0,0}, {0,0} },
					SDL_Vertex{ static_cast<float>(knob_rect.x + knob_rect.w - 5),
					static_cast<float>(knob_rect.y + knob_height / 5), {0,0,0,0}, {0,0} },
					SDL_Vertex{ static_cast<float>(knob_rect.x + knob_rect.w - 5),
					static_cast<float>(knob_rect.y + knob_height * 4 / 5), {0,0,0,0}, {0,0} },
			};
			break;
		}
		case PanelState::open:
		case PanelState::closing:
		{
			arrow_verts =
				std::array<SDL_Vertex, 3>{
				SDL_Vertex{ static_cast<float>(knob_rect.x + knob_rect.w - 5) , static_cast<float>(knob_rect.y + knob_height / 2)
				, {0,0,0,0}, {0,0} },
					SDL_Vertex{ static_cast<float>(knob_rect.x + 5),
					static_cast<float>(knob_rect.y + knob_height / 5), {0,0,0,0}, {0,0} },
					SDL_Vertex{ static_cast<float>(knob_rect.x + 5),
					static_cast<float>(knob_rect.y + knob_height * 4 / 5), {0,0,0,0}, {0,0} },
			};
			break;
		}
		}
	}
	break;

	case PanelSide::left:
		assert(0);
		break;
	}
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
	SDL_RenderGeometry(renderer, nullptr, arrow_verts.data(), 3, nullptr, 0);
}

void node::SidePanel::RepositionWidget()
{
	switch (m_side)
	{
	case PanelSide::right:
	{
		auto width = GetRect().w - knob_width;
		SetRect({ static_cast<float>(m_corner_position - width * easeOut(m_expand_percent) - knob_width),
			0.0f, GetRect().w, GetRect().h });
		if (m_contained_widget)
		{
			m_contained_widget->SetRect(CalculateChildWidgetRect());
		}
		break;
	}
	case PanelSide::left:
		assert(false);
		break;
	}
}

