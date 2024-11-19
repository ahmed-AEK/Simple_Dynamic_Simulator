#pragma once

#include "toolgui/Widget.hpp"
#include "SDL_Framework/Utility.hpp"
#include "NodeModels/Observer.hpp"
#include <variant>

namespace node
{

class TabbedView;
class TabBar;

class TabButton : public Widget
{
public:
	TabButton(TTF_Font* font, const SDL_FRect& rect, TabBar* parent);
	void Draw(SDL_Renderer* renderer) override;
	void SetText(std::string name);
	void SetActive(bool value = true);
	bool GetActive() const { return m_active; }
protected:
	void OnMouseOut() override;
	void OnMouseIn() override;
	void OnMouseMove(MouseHoverEvent& e) override;
	MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;
	MI::ClickEvent OnLMBUp(MouseButtonEvent& e) override;
private:
	SDL_FRect GetXBtnRect() const;
	TextPainter m_tab_text;
	TextPainter m_X_painter;
	TabBar* m_parent;
	bool m_active = false;
	bool m_mouse_hovered = false;
	bool m_exit_initiated = false;
};

class TabBar : public Widget
{
public:
	TabBar(TTF_Font* font, const SDL_FRect& rect, TabbedView* parent);
	void Draw(SDL_Renderer* renderer) override;
	void AddTab(std::string name);
	void DeleteTab(size_t index);
	int GetTabWidth() const { return tab_width; }
	void SetActiveTabIndex(size_t index);
	void ButtonClicked(TabButton* btn);
	void ButtonXClicked(TabButton* btn);
protected:
	void OnSetRect(const SDL_FRect& rect) override;
	Widget* OnGetInteractableAtPoint(const SDL_FPoint& point) override;

private:
	void ReCalcLayout();
	TTF_Font* m_font;
	TabbedView* m_parent;
	std::vector<std::unique_ptr<TabButton>> m_buttons;
	size_t m_active_tab = 0;
	static constexpr int tab_width = 100;
};

struct TabIndexChangeEvent
{
	size_t old_tab_idx;
	size_t new_tab_idx;
	Widget* old_tab;
	Widget* new_tab;
};

struct TabCloseRequestEvent
{
	size_t tab_idx;
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
	TabbedView(TTF_Font* font, const SDL_FRect& rect, Widget* parent);
	void AddTab(std::string tab_name, std::unique_ptr<Widget> widget);
	void Draw(SDL_Renderer* renderer) override;
	void SetCurrentTabIndex(size_t index);
	void RequestDeleteTab(size_t index);
	void DeleteTab(size_t index);
protected:
	void OnSetRect(const SDL_FRect& rect) override;
	Widget* OnGetInteractableAtPoint(const SDL_FPoint& point) override;

private:
	int GetTabsBarHeight() const;
	struct TabData
	{
		std::string name;
		std::unique_ptr<Widget> widget;
	};
	TabBar m_bar;
	std::vector<TabData> m_tabs;
	size_t m_current_tab_index = 0;
};
}