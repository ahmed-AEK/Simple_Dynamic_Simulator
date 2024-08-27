#pragma once

#include "toolgui/Widget.hpp"

namespace node
{

class Dialog : public Widget
{
public:
	enum class ScreenResizeStrategy
	{
		FixedPosition,
		Center,
	};

	Dialog(std::string title, const SDL_Rect& rect, Scene* parent);
	void Draw(SDL_Renderer* renderer) override;
	const std::string& GetTitle() const { return m_title; };
	void SetTitle(std::string title) { m_title = title; }

	ScreenResizeStrategy GetResizeStrategy() const { return m_resize_strategy; }
	void SetResizeStrategy(ScreenResizeStrategy strategy) { m_resize_strategy = strategy; }
protected:
	void OnMouseMove(const SDL_Point& current_mouse_point);
	MI::ClickEvent OnLMBDown(const SDL_Point& current_mouse_point);
	MI::ClickEvent OnLMBUp(const SDL_Point& current_mouse_point);
	void OnMouseOut() override;

private:
	void DrawTitle(SDL_Renderer* renderer, const SDL_Point& start);
	void DrawXButton(SDL_Renderer* renderer, const SDL_Rect& rect);
	SDL_Rect GetTitleBarRect() const;
	SDL_Rect GetXButtonRect() const;
	ScreenResizeStrategy m_resize_strategy = ScreenResizeStrategy::FixedPosition;
	std::string m_title;
	SDL_Point m_drag_edge_start_position{ 0,0 };
	SDL_Point m_drag_mouse_start_position{ 0,0 };
	bool b_being_dragged = false;
	bool b_being_closed = false;
	bool b_mouse_on_close = false;
};
}