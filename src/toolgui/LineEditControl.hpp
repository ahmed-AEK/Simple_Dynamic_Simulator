#pragma once

#include "toolgui/Widget.hpp"
#include "SDL_Framework/Utility.hpp"

namespace node
{
class LineEditControl : public Widget
{
public:
	LineEditControl(std::string initial_value, const SDL_Rect& rect, Scene* parent);
	void Draw(SDL_Renderer* renderer) override;
	const std::string& GetValue() const { return m_value; }
protected:
	MI::ClickEvent OnLMBDown(const SDL_Point& current_mouse_point);
	void OnChar(int32_t key) override;
	void OnKeyPress(int32_t key) override;
	virtual void OnKeyboardFocusIn();
	virtual void OnKeyboardFocusOut();
private:
	void ReCalculateCursorPixelPosition();
	std::string m_value;
	TextPainter m_painter;
	size_t m_cursor_position;
	int m_cursor_pixel_position{ 1 };
	bool m_focused = false;
};
}
