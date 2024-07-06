#pragma once

#include "toolgui/Widget.hpp"
#include <cassert>
#include <array>

namespace node
{

	class SidePanel : public Widget
	{
	public:

		enum PanelSide
		{
			left,
			right,
		};

		SidePanel(PanelSide side, SDL_Rect rect, Scene* parent)
			: m_side(side), Widget(rect, parent)
		{
			RepositionWidget();
		}

		void Expand() noexcept
		{
			if (m_state == PanelState::open)
			{
				return;
			}
			m_last_action_time = SDL_GetTicks64();
			m_state = PanelState::openning;
			m_updateTaskId = GetScene()->AddUpdateTask({ GetMIHandlePtr(), [this]() { this->UpdatePanelMotion(); } });
		}

		void Retract() noexcept
		{
			if (m_state == PanelState::closed)
			{
				return;
			}
			m_last_action_time = SDL_GetTicks64();
			m_state = PanelState::closing;
			m_updateTaskId = GetScene()->AddUpdateTask({ GetMIHandlePtr(), [this]() { this->UpdatePanelMotion(); } });
		}

		void SetWidget(std::unique_ptr<Widget> widget)
		{
			m_contained_widget = std::move(widget);
			if (m_contained_widget)
			{
				m_contained_widget->SetRect(CalculateChildWidgetRect());
			}
		}
		void Draw(SDL_Renderer* renderer) override
		{
			SDL_Rect draw_area = GetRect();
			draw_area.x += knob_width;
			draw_area.w -= knob_width;
			// draw main widget part
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderFillRect(renderer, &draw_area);
			SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
			SDL_Rect inner_rect{ draw_area.x + 2, draw_area.y + 2, draw_area.w - 4, draw_area.h - 4 };
			SDL_RenderFillRect(renderer, &inner_rect);

			// draw side knob
			DrawKnob(renderer);

			// draw inner widget
			if (m_state != PanelState::closed && m_contained_widget)
			{
				m_contained_widget->Draw(renderer);
			}

		}
		MI::ClickEvent OnLMBDown(const SDL_Point& current_mouse_point) override
		{
			UNUSED_PARAM(current_mouse_point);
			SDL_Rect knobRect = GetKnobRect();
			if (!SDL_PointInRect(&current_mouse_point, &knobRect)) { return MI::ClickEvent::NONE; }
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
			}
			return MI::ClickEvent::NONE;
		}

		void UpdateWindowSize(const SDL_Rect& rect)
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

		void SetSide(PanelSide side) noexcept
		{
			m_side = side;
		}

		const PanelSide& GetSide() const noexcept
		{
			return m_side;
		}
	protected:
		Widget* OnGetInteractableAtPoint(const SDL_Point& point) override
		{
			auto&& knobRect = GetKnobRect();
			if (SDL_PointInRect(&point, &knobRect))
			{
				return this;
			}
			if (m_contained_widget && 
				SDL_PointInRect(&point, &m_contained_widget->GetRect()))
			{
				return m_contained_widget.get();
			}
			else
			{
				SDL_Rect inner_rect = GetRect();
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
				if (SDL_PointInRect(&point, &inner_rect))
				{
					return this;
				}

			}
			return nullptr;
		}

	private:

		void UpdatePanelMotion()
		{
			if (PanelState::openning == m_state)
			{
				auto passed_ticks = SDL_GetTicks64() - m_last_action_time;
				auto passed_time = passed_ticks / TICKS_PER_SECOND;
				m_expand_percent = passed_time / TRANSITION_TIME;
				if (m_expand_percent >= 1)
				{
					m_expand_percent = 1;
					m_state = PanelState::open;
					GetScene()->RemoveUpdateTask(m_updateTaskId);
					m_updateTaskId = -1;
				}
				RepositionWidget();
			}
			if (PanelState::closing == m_state)
			{
				auto passed_ticks = SDL_GetTicks64() - m_last_action_time;
				auto passed_time = passed_ticks / TICKS_PER_SECOND;
				m_expand_percent = 1 - passed_time / TRANSITION_TIME;
				if (m_expand_percent <= 0)
				{
					m_expand_percent = 0;
					m_state = PanelState::closed;
					GetScene()->RemoveUpdateTask(m_updateTaskId);
					m_updateTaskId = -1;
				}
				RepositionWidget();
			}
		}

		SDL_Rect CalculateChildWidgetRect()
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
		SDL_Rect GetKnobRect()
		{
			switch (m_side)
			{
			case PanelSide::right:
				return { GetRect().x, GetRect().y + GetRect().h / 2 - knob_height / 2, knob_width, knob_height};
			case PanelSide::left:
				assert(false);
				return {};
			}
			return {};
		}
		void DrawKnob(SDL_Renderer* renderer)
		{
			SDL_Point knob_start{};
			SDL_Point knob_end{};
			switch (m_side)
			{
			case PanelSide::right:
				knob_start = { GetRect().x, GetRect().h / 2 - knob_height / 2 };
				knob_end = { GetRect().x + knob_width, GetRect().h / 2 + knob_height / 2 };
				break;
			case PanelSide::left:
				assert(false);
				break;
			}
			SDL_Rect knob_rect = GetKnobRect();
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderFillRect(renderer, &knob_rect);
			SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
			SDL_Rect inner_rect{ knob_rect.x + 2, knob_rect.y + 2, knob_rect.w - 4, knob_rect.h - 4 };
			SDL_RenderFillRect(renderer, &inner_rect);

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
			
			SDL_RenderGeometry(renderer, nullptr, arrow_verts.data(), 3, nullptr, 0);
		}
		void RepositionWidget()
		{
			switch (m_side)
			{
			case PanelSide::right:
			{
				auto width = GetRect().w - knob_width;
				SetRect({ static_cast<int>(m_corner_position - width * m_expand_percent - knob_width),
					0, GetRect().w, GetRect().h });
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
		enum PanelState
		{
			closed,
			open,
			openning,
			closing,
		};

		std::unique_ptr<Widget> m_contained_widget;
		PanelSide m_side = PanelSide::right;
		PanelState m_state = PanelState::closed;

		double m_expand_percent = 0;
		uint64_t m_last_action_time = 0;
		int64_t m_corner_position = 0;
		int64_t m_updateTaskId = -1;

		static constexpr double TICKS_PER_SECOND = 1000;
		static constexpr double TRANSITION_TIME = 0.15;
		static constexpr int knob_width = 50;
		static constexpr int knob_height = 100;
		static constexpr int widget_margin = 2;
	};


	class TestWidget:public Widget
	{
	public:
		using Widget::Widget;
		void Draw(SDL_Renderer* renderer) override
		{
			SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
			SDL_RenderFillRect(renderer, &GetRect());
		}

	protected:
		MI::ClickEvent OnLMBDown(const SDL_Point& current_mouse_point) override
		{
			UNUSED_PARAM(current_mouse_point);
			SDL_Log("Clicked!");
			return MI::ClickEvent::CLICKED;
		}

	};
}