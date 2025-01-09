#pragma once

#include "toolgui/Widget.hpp"
#include "SDL_Framework/Utility.hpp"

#include <functional>
#include <variant>
namespace node
{
class Dialog;

class DialogButton : public Widget
{
public:
	DialogButton(std::string text, TTF_Font* font, std::function<void()> OnClick, const WidgetSize& size, Widget* scene);

protected:
	void OnDraw(SDL::Renderer& renderer) override;
	void OnMouseOut(MouseHoverEvent& e) override;
	MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;
	MI::ClickEvent OnLMBUp(MouseButtonEvent& e) override;
private:
	TextPainter m_text_painter;
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

	DialogControl(const WidgetSize& size, Dialog* parent);
	SDL_FRect GetSizeHint() const { return m_size_hint; }
	void SetSizeHint(const SDL_FRect& rect) { m_size_hint = rect; }
	SizingMode GetSizingMode() const { return m_sizingMode; }
	void SetSizingMode(SizingMode mode) { m_sizingMode = mode; }
private:
	SDL_FRect m_size_hint{ 0,0,0,0 };
	SizingMode m_sizingMode = SizingMode::constantSize;
};

class ToolBar;

class Dialog : public Widget
{
public:
	enum class ScreenResizeStrategy: unsigned char
	{
		FixedPosition,
		Center,
	};

	Dialog(std::string title, const WidgetSize& size, Scene* parent);
	~Dialog() override;
	const std::string& GetTitle() const { return m_title; };
	void SetTitle(std::string title) { m_title = title; m_title_painter.SetText(title); }

	ScreenResizeStrategy GetResizeStrategy() const { return m_resize_strategy; }
	void SetResizeStrategy(ScreenResizeStrategy strategy) { m_resize_strategy = strategy; }

	void AddControl(std::unique_ptr<DialogControl> control, int position = -1);
	void AddButton(std::string title, std::function<void()> callback);

	void TriggerClose() { OnClose(); }
	void TriggerOk() { OnOk(); }
	
	SDL_FRect GetTitleBarRect() const;
	
	void SetToolbar(std::unique_ptr<ToolBar> toolbar);
	
	void SetResizeable(bool value = true) { m_resizable = value; }
	
	Scene* GetScene() const { return m_scene; }
	void SetScene(Scene* scene) { m_scene = scene; }

	Widget* GetFocusable() override;
	bool BeingDragged() const;
	void StopDrag();
protected:
	void OnDraw(SDL::Renderer& renderer) override;

	void OnMouseMove(MouseHoverEvent& e) override;
	MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;
	MI::ClickEvent OnLMBUp(MouseButtonEvent& e) override;
	void OnMouseOut(MouseHoverEvent& e) override;
	Widget* OnGetInteractableAtPoint(const SDL_FPoint& point) override;
	void OnSetSize(const WidgetSize& size) override;
	virtual void OnClose();
	virtual void OnOk();
	bool OnKeyPress(KeyboardEvent& e) override;

private:
	void RepositionControls();
	void RepositionButtons();
	void ResizeToFitChildren();
	void DrawTitle(SDL_Renderer* renderer, const SDL_FPoint& start);
	void DrawXButton(SDL_Renderer* renderer, const SDL_FRect& rect);
	void DrawOutline(SDL_Renderer* renderer, const SDL_FRect& rect);
	SDL_FRect GetXButtonRect() const;
	SDL_FRect GetResizeGripRect() const;
	WidgetSize CalculateMinSize() const;

	std::vector<std::unique_ptr<DialogControl>> m_controls;
	std::vector<std::unique_ptr<DialogButton>> m_buttons;
	std::unique_ptr<ToolBar> m_toolbar;
	TextPainter m_title_painter;
	TextPainter m_X_painter;

	static constexpr float ButtonHeight{ 25.0f };
	static constexpr float ButtonsMargin{ 6.0f };
	static constexpr float ControlsMargin{ 6.0f };
	static constexpr float MinWidth{ 300.0f };
	static constexpr float top_resize_grip_height{ 3.0f };

	struct TitleDrag
	{
		// both points relative to parent
		SDL_FPoint drag_edge_start_position{ 0,0 }; 
		SDL_FPoint drag_mouse_start_position{ 0,0 };
	};
	struct ResizeDrag
	{
		enum class DragMode
		{
			grip,
			top,
		};
		SDL_FPoint drag_edge_start_position{ 0,0 };  // relative to parent
		WidgetSize min_size;
		DragMode mode;
	};
	using DragData = std::variant<std::monostate, TitleDrag, ResizeDrag>;

	DragData m_dragData;
	std::string m_title;
	Scene* m_scene;
	size_t m_var_height_elements = 0;
	bool b_being_closed = false;
	bool b_mouse_on_close = false;
	ScreenResizeStrategy m_resize_strategy = ScreenResizeStrategy::FixedPosition;
	bool m_resizable = false;
	float m_excess_height = 0;
};
}