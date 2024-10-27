#pragma once

#include "toolgui/Widget.hpp"
#include <cassert>
#include <array>
#include "SDL_Framework/Utility.hpp"

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

		SidePanel(PanelSide side, const SDL_Rect& rect, Widget* parent);

		void Expand() noexcept;
		void Retract() noexcept;
		void UpdateWindowSize(const SDL_Rect& rect);

		void SetWidget(std::unique_ptr<Widget> widget);

		void Draw(SDL_Renderer* renderer) override;

		void SetSide(PanelSide side) noexcept {	m_side = side; }
		const PanelSide& GetSide() const noexcept {	return m_side; }

	protected:
		Widget* OnGetInteractableAtPoint(const SDL_Point& point) override;
		MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;

	private:

		void UpdatePanelMotion();
		SDL_Rect CalculateChildWidgetRect();
		void RepositionWidget();

		SDL_Rect GetKnobRect();
		void DrawKnob(SDL_Renderer* renderer);

		enum PanelState
		{
			closed,
			open,
			openning,
			closing,
		};
		RoundRectPainter m_outer_painter;
		RoundRectPainter m_inner_painter;

		std::unique_ptr<Widget> m_contained_widget;
		PanelSide m_side = PanelSide::right;
		PanelState m_state = PanelState::closed;

		double m_expand_percent = 0;
		uint64_t m_last_action_time = 0;
		int64_t m_corner_position = 0;
		int64_t m_updateTaskId = 0;

		static constexpr double TICKS_PER_SECOND = 1000;
		static constexpr double TRANSITION_TIME = 0.25;
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
		MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override
		{
			UNUSED_PARAM(e);
			SDL_Log("Clicked!");
			return MI::ClickEvent::CLICKED;
		}

	};
}