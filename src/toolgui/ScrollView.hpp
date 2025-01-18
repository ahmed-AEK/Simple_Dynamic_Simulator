#pragma once

#include "toolgui/Widget.hpp"
#include "SDL_Framework/Utility.hpp"
#include "NodeModels/Observer.hpp"

namespace node
{
class ScrollView;

namespace scrolltools
{

class ScrollBar : public Widget
{
public:
	static constexpr float scrollbar_width = 10;
	ScrollBar(const WidgetSize& size, ScrollView* parent);
	void SetScrollInfo(float size, float bar_extent);
	void SetScrollPosition(float position);
	float GetScrollPosition() const { return m_bar_pos; }
	float GetMaxPosition() const { float out = m_bar_extent - m_bar_size; return out < 0 ? 0 : out; }
protected:
	void OnDraw(SDL::Renderer& renderer) override;
	MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;
	MI::ClickEvent OnLMBUp(MouseButtonEvent& e) override;	
	void OnMouseMove(MouseHoverEvent& e) override;
private:
	SDL_FRect GetBarRect() const;
	ScrollView* m_parent_view = nullptr;
	RoundRectPainter m_painter;
	float m_bar_size = 10;
	float m_bar_extent = 5;
	float m_bar_pos = 0;
	float m_start_drag_top_position = 0;
	float m_start_drag_mouse_position = 0;
	bool m_being_dragged = 0;
};

}

class ScrollView: public Widget
{
public:
	ScrollView(const WidgetSize& size, Widget* parent);
	void SetWidget(std::unique_ptr<Widget> widget);
	void RequestPosition(float new_position);
protected:
	bool OnScroll(const double amount, const SDL_FPoint& p) override;
	void OnSetSize(const WidgetSize& size) override;
private:
	WidgetSize GetScrollBarSize() const;
	SDL_FPoint GetScrollBarPosition() const;
	WidgetSize GetContainedWidgetSize() const;
	void SetScrollInfo(float size, float max);

	scrolltools::ScrollBar m_scrollbar;
	std::unique_ptr<Widget> m_contained_widget;
};

}