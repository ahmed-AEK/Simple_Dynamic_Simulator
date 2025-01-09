#pragma once

#include "toolgui/Widget.hpp"
#include <cassert>
#include <array>
#include "SDL_Framework/Utility.hpp"
#include "SDL_Framework/SVGRasterizer.hpp"

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

		SidePanel(PanelSide side, const WidgetSize& size, Widget* parent);

		void Expand() noexcept;
		void Retract() noexcept;
		void UpdateWindowSize(const WidgetSize& rect);

		void SetWidget(std::unique_ptr<Widget> widget);

		void OnDraw(SDL::Renderer& renderer) override;

		void SetSide(PanelSide side) noexcept {	m_side = side; }
		const PanelSide& GetSide() const noexcept {	return m_side; }

	protected:
		Widget* OnGetInteractableAtPoint(const SDL_FPoint& point) override;
		MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;
		void OnSetSize(const WidgetSize& size) override;
	private:

		void UpdatePanelMotion();
		SDL_FRect CalculateChildWidgetRect();
		void RepositionWidget();

		SDL_FRect GetKnobRect();
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
		SVGRasterizer m_knob_drawer_left;
		SVGRasterizer m_knob_drawer_right;

		std::unique_ptr<Widget> m_contained_widget;
		PanelSide m_side = PanelSide::right;
		PanelState m_state = PanelState::closed;

		double m_expand_percent = 0;
		uint64_t m_last_action_time = 0;
		float m_corner_position = 0;
		int64_t m_updateTaskId = 0;

		static constexpr double TICKS_PER_SECOND = 1000;
		static constexpr double TRANSITION_TIME = 0.25;
		static constexpr int knob_width = 40;
		static constexpr int knob_height = 80;
		static constexpr int widget_margin = 2;
	};

}