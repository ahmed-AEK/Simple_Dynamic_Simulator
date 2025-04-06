#pragma once

#include "toolgui/Widget.hpp"
#include <cassert>
#include <array>
#include "SDL_Framework/Utility.hpp"
#include "SDL_Framework/SVGRasterizer.hpp"
#include "NodeModels/Observer.hpp"
#include "toolgui/ButtonWidget.hpp"

namespace node
{
	enum PanelSide
	{
		right,
		bottom,
	};
	class SidePanel;
	struct PanelCloseRequest
	{
		SidePanel* panel{};
		PanelSide side;
	};

	class SidePanel : public Widget, public MultiPublisher<PanelCloseRequest>
	{
	public:

		using PanelSide = node::PanelSide;

		SidePanel(PanelSide side, TTF_Font* font, const WidgetSize& size, Widget* parent);

		void SetWidget(std::unique_ptr<Widget> widget);
		void SetTitle(std::string title);
		const std::string& GetTitle() const { return m_title_painter.GetText(); }

		void OnDraw(SDL::Renderer& renderer) override;

		void SetSide(PanelSide side) noexcept {	m_side = side; }
		const PanelSide& GetSide() const noexcept {	return m_side; }

	protected:
		MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;
		void OnSetSize(const WidgetSize& size) override;
	private:

		SDL_FRect CalculateChildWidgetRect();
		void OnRequestClosePanel();
		SDL_FPoint GetCloseBtnPosition();
		static ButtonWidget CreateCloseButton(SidePanel& panel, PanelSide side);

		ButtonWidget m_close_btn;
		RoundRectPainter m_outer_painter;
		RoundRectPainter m_inner_painter;
		TextPainter m_title_painter;
		std::unique_ptr<Widget> m_contained_widget;
		PanelSide m_side = PanelSide::right;
		static constexpr float widget_margin = 8;
		static constexpr float TitleHeight = 40;
	};

}