#pragma once

#include "toolgui/Widget.hpp"
#include "SDL_Framework/Utility.hpp"

namespace node
{
class LineEditControl : public Widget
{
public:
	LineEditControl(std::string initial_value, const SDL_Rect& rect, Widget* parent);
	void Draw(SDL_Renderer* renderer) override;
	const std::string& GetValue() const { return m_value; }
protected:
	MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;
	void OnChar(TextInputEvent& e) override;
	void OnKeyPress(KeyboardEvent& e) override;
	virtual void OnKeyboardFocusIn() override;
	virtual void OnKeyboardFocusOut() override;
private:
	void ReCalculateCursorPixelPosition();
	std::string m_value;
	TextPainter m_painter;
	size_t m_cursor_position;
	int m_cursor_pixel_position{ 1 };
	bool m_focused = false;
};
}
