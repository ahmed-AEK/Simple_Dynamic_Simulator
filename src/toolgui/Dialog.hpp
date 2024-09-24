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
	void Draw(SDL_Renderer* renderer) override;

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
	enum class SizingMode
	{
		constantSize,
		expanding,
	};

	DialogControl(const SDL_Rect& rect, Scene* parent);
	SDL_Rect GetSizeHint() const { return m_size_hint; }
	void SetSizeHint(const SDL_Rect& rect) { m_size_hint = rect; }
	SizingMode GetSizingMode() const { return m_sizingMode; }
	void SetSizingMode(SizingMode mode) { m_sizingMode = mode; }
private:
	SDL_Rect m_size_hint{ 0,0,0,0 };
	SizingMode m_sizingMode = SizingMode::constantSize;
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
	~Dialog() override;
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
	void SetResizeable(bool value = true) { m_resizable = value; }
protected:
	void OnMouseMove(const SDL_Point& current_mouse_point) override;
	MI::ClickEvent OnLMBDown(const SDL_Point& current_mouse_point) override;
	MI::ClickEvent OnLMBUp(const SDL_Point& current_mouse_point) override;
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
	SDL_Rect GetResizeGripRect() const;
	bool BeingDragged() const;
	SDL_Point CalculateMinSize() const;

	std::vector<std::unique_ptr<DialogControl>> m_controls;
	std::vector<std::unique_ptr<DialogButton>> m_buttons;
	std::unique_ptr<ToolBar> m_toolbar;
	TextPainter m_title_painter;

	static constexpr int ButtonHeight = 35;
	static constexpr int ButtonsMargin = 10;
	static constexpr int ControlsMargin = 10;
	static constexpr int MinWidth = 300;
	static constexpr int top_resize_grip_height = 3;

	struct TitleDrag
	{
		SDL_Point drag_edge_start_position{ 0,0 };
		SDL_Point drag_mouse_start_position{ 0,0 };
	};
	struct ResizeDrag
	{
		enum class DragMode
		{
			grip,
			top,
		};
		SDL_Point drag_edge_start_position{ 0,0 };
		SDL_Point min_size;
		DragMode mode;
	};
	using DragData = std::variant<std::monostate, TitleDrag, ResizeDrag>;

	DragData m_dragData;
	std::string m_title;
	size_t m_var_height_elements = 0;
	bool b_being_closed = false;
	bool b_mouse_on_close = false;
	ScreenResizeStrategy m_resize_strategy = ScreenResizeStrategy::FixedPosition;
	bool m_resizable = false;
	int m_excess_height = 0;
};
}