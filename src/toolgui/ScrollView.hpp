#pragma once

#include "toolgui/Widget.hpp"
#include "SDL_Framework/Utility.hpp"
#include "NodeModels/Observer.hpp"

namespace node
{
class ScrollViewBase;

namespace scrolltools
{

class ScrollBar : public Widget
{
public:
	static constexpr float scrollbar_width = 10;
	ScrollBar(const WidgetSize& size, ScrollViewBase* parent);
	void SetScrollInfo(float size, float max);
	void SetScrollPosition(float position);
	float GetScrollPosition() const { return m_bar_pos; }
	float GetMaxPosition() const { float out = m_bar_extent - m_bar_size; return out < 0 ? 0 : out; }
	float GetScrollSize() const { return m_bar_size;  }
protected:
	void OnDraw(SDL::Renderer& renderer) override;
	MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;
	MI::ClickEvent OnLMBUp(MouseButtonEvent& e) override;	
	void OnMouseMove(MouseHoverEvent& e) override;
private:
	SDL_FRect GetBarRect() const;
	ScrollViewBase* m_parent_view = nullptr;
	RoundRectPainter m_painter;
	float m_bar_size = 10;
	float m_bar_extent = 5;
	float m_bar_pos = 0;
	float m_start_drag_top_position = 0;
	float m_start_drag_mouse_position = 0;
	bool m_being_dragged = 0;
};

}

class ScrollViewBase : public Widget
{
public:
	ScrollViewBase(const WidgetSize& size, Widget* parent);
	void RequestPosition(float new_position);
	float GetScrollStrength() const { return m_scroll_strength; }
	void SetScrollStrength(float strength) { m_scroll_strength = strength; }
	WidgetSize GetContainedAreaSize() const;

	struct ScrollInfo
	{
		float page_size;
		float max_position;
		float position;
	};
	ScrollInfo GetScrollInfo() const;

protected:
	void SetScrollInfo(float page_size, float max_position, float position);
	void SetScrollPosition(float position);
	void OnSetSize(const WidgetSize& size) override;
	virtual void OnPositionRequested(float new_position);
	bool OnScroll(const double amount, const SDL_FPoint& p) override;

private:
	WidgetSize GetScrollBarSize() const;
	SDL_FPoint GetScrollBarPosition() const;
	scrolltools::ScrollBar m_scrollbar;
	float m_scroll_strength = 30;
};

class ScrollView: public ScrollViewBase
{
public:
	ScrollView(const WidgetSize& size, Widget* parent);
	void SetWidget(std::unique_ptr<Widget> widget);
protected:
	void OnSetSize(const WidgetSize& size) override;
	virtual void OnPositionRequested(float new_position);
private:
	std::unique_ptr<Widget> m_contained_widget;

};

}