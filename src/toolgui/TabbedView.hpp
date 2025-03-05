#pragma once

#include "toolgui/Widget.hpp"
#include "SDL_Framework/Utility.hpp"
#include "NodeModels/Observer.hpp"
#include "toolgui/StackedWidget.hpp"

#include <variant>

namespace node
{

class TabbedView;
class TabBar;

class TabButton : public Widget
{
public:
	TabButton(TTF_Font* font, const WidgetSize& size, TabBar* parent);
	void SetText(std::string name);
	void SetActive(bool value = true);
	bool IsActiveBtn() const { return m_active; }
protected:
	void OnDraw(SDL::Renderer& renderer) override;
	void OnMouseOut(MouseHoverEvent& e) override;
	void OnMouseIn(MouseHoverEvent& e) override;
	void OnMouseMove(MouseHoverEvent& e) override;
	MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;
	MI::ClickEvent OnLMBUp(MouseButtonEvent& e) override;
private:
	SDL_FRect GetXBtnRect() const;
	float GetXBtnStart() const;
	TextPainter m_tab_text;
	TextPainter m_X_painter;
	RoundRectPainter m_outer_painter;
	RoundRectPainter m_inner_painter;
	RoundRectPainter m_X_btn_painter_outer;
	RoundRectPainter m_X_btn_painter_inner;
	TabBar* m_parent;
	bool m_active = false;
	bool m_mouse_hovered = false;
	bool m_exit_initiated = false;
};

class TabBar : public Widget
{
public:
	TabBar(TTF_Font* font, const WidgetSize& size, TabbedView* parent);
	void AddTab(std::string name);
	void DeleteTab(int32_t index);
	int GetTabWidth() const { return tab_width; }
	void SetActiveTabIndex(int32_t index);
	void ButtonClicked(TabButton* btn);
	void ButtonXClicked(TabButton* btn);
	int32_t TabsCount() const { return static_cast<int32_t>(m_buttons.size()); }
	void SetTabName(int32_t idx, std::string_view name);

	static constexpr auto npos = StackedWidget::npos;
protected:
	void OnDraw(SDL::Renderer& renderer) override;
	void OnMouseMove(MouseHoverEvent& e) override;
	MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;
	MI::ClickEvent OnLMBUp(MouseButtonEvent& e) override;

private:
	void ReCalcLayout();


	struct TabButtonDragData
	{
		HandlePtrS<TabButton, Widget> btn_ptr;
	};

	TTF_Font* m_font;
	TabbedView* m_parent;
	std::vector<std::unique_ptr<TabButton>> m_buttons;
	std::vector<TabButton*> m_buttons_preview_order;
	int32_t m_active_tab = npos;
	std::optional<TabButtonDragData> m_current_drag_data;
	static constexpr int tab_width = 115;
};

struct TabIndexChangeEvent
{
	int32_t old_tab_idx;
	int32_t new_tab_idx;
	Widget* new_tab;
};

struct TabCloseRequestEvent
{
	int32_t tab_idx;
	Widget* widget;
};

struct TabCloseEvent
{
	size_t tab_idx;
};

struct TabsChangeEvent
{
	std::variant<TabIndexChangeEvent, TabCloseRequestEvent, TabCloseEvent> e;
};

class TabbedView : public Widget, public MultiPublisher<TabsChangeEvent>
{
public:
	TabbedView(TTF_Font* font, const WidgetSize& size, Widget* parent);
	int32_t AddTab(std::string tab_name, std::unique_ptr<Widget> widget);
	void SetCurrentTabIndex(Widget* ptr);
	void SetCurrentTabIndex(int32_t index);
	void RequestDeleteTab(int32_t index);
	void DeleteTab(int32_t index);
	Widget* GetTabWidget(int32_t index);
	int32_t GetWidgetIndex(Widget* widget);
	int32_t TabsCount() const { return m_stacked_widget.WidgetsCount(); }

	void SetTabName(int32_t idx, std::string_view name);

	static constexpr auto npos = StackedWidget::npos;
protected:
	void OnSetSize(const WidgetSize& size) override;
	void OnDraw(SDL::Renderer& renderer) override;
private:
	int GetTabsBarHeight() const;
	StackedWidget m_stacked_widget;
	std::vector<std::string> m_tab_names;
	TabBar m_bar;
	RoundRectPainter m_outer_bg_painter;
	RoundRectPainter m_inner_bg_painter;
};
}