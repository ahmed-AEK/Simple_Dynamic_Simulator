#pragma once

#include "toolgui/Widget.hpp"
#include <functional>
#include "SDL_Framework/Utility.hpp"

namespace node
{

class DialogButton : public Widget
{
public:
	DialogButton(std::string text, std::function<void()> OnClick, const SDL_Rect& rect, Scene* scene);
	void Draw(SDL_Renderer* renderer);

protected:
	void OnMouseOut() override;
	MI::ClickEvent OnLMBDown(const SDL_Point& current_mouse_point) override;
	MI::ClickEvent OnLMBUp(const SDL_Point& current_mouse_point) override;
private:
	std::string m_text;
	std::function<void()> m_onClick;
	bool b_being_clicked = false;
};

class DialogControl : public Widget
{
public:
	using Widget::Widget;
private:

};

class ToolBar;

class Dialog : public Widget
{
public:
	enum class ScreenResizeStrategy
	{
		FixedPosition,
		Center,
	};

	Dialog(std::string title, const SDL_Rect& rect, Scene* parent);
	~Dialog();
	void Draw(SDL_Renderer* renderer) override;
	const std::string& GetTitle() const { return m_title; };
	void SetTitle(std::string title) { m_title = title; m_title_painter.SetText(title); }

	ScreenResizeStrategy GetResizeStrategy() const { return m_resize_strategy; }
	void SetResizeStrategy(ScreenResizeStrategy strategy) { m_resize_strategy = strategy; }

	void AddControl(std::unique_ptr<DialogControl> control, int position = -1);
	void AddButton(std::string title, std::function<void()> callback);
	void TriggerClose() { OnClose(); }
	void TriggerOk() { OnOk(); }
	SDL_Rect GetTitleBarRect() const;
	void SetToolbar(std::unique_ptr<ToolBar> toolbar);
protected:
	void OnMouseMove(const SDL_Point& current_mouse_point);
	MI::ClickEvent OnLMBDown(const SDL_Point& current_mouse_point);
	MI::ClickEvent OnLMBUp(const SDL_Point& current_mouse_point);
	void OnMouseOut() override;
	Widget* OnGetInteractableAtPoint(const SDL_Point& point) override;
	void OnSetRect(const SDL_Rect& rect) override;
	virtual void OnClose();
	virtual void OnOk();

private:
	void RepositionControls();
	void RepositionButtons();
	void ResizeToFitChildren();
	void DrawTitle(SDL_Renderer* renderer, const SDL_Point& start);
	void DrawXButton(SDL_Renderer* renderer, const SDL_Rect& rect);
	void DrawOutline(SDL_Renderer* renderer, const SDL_Rect& rect);
	SDL_Rect GetXButtonRect() const;

	std::vector<std::unique_ptr<DialogControl>> m_controls;
	std::vector<std::unique_ptr<DialogButton>> m_buttons;
	std::unique_ptr<ToolBar> m_toolbar;
	TextPainter m_title_painter;

	std::string m_title;
	SDL_Point m_drag_edge_start_position{ 0,0 };
	SDL_Point m_drag_mouse_start_position{ 0,0 };
	bool b_being_dragged = false;
	bool b_being_closed = false;
	bool b_mouse_on_close = false;
	ScreenResizeStrategy m_resize_strategy = ScreenResizeStrategy::FixedPosition;

	static constexpr int ButtonHeight = 35;
	static constexpr int ButtonsMargin = 10;
	static constexpr int ControlsMargin = 10;
	static constexpr int MinWidth = 300;
};
}