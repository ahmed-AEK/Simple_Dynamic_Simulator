#pragma once

#include "toolgui/Widget.hpp"
#include "SDL_Framework/Utility.hpp"

namespace node
{
class LineEditControl : public Widget
{
public:
	LineEditControl(std::string initial_value, const SDL_FRect& rect, Widget* parent);
	void Draw(SDL_Renderer* renderer) override;
	const std::string& GetValue() const { return m_value; }
	~LineEditControl() override;
protected:
	MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;
	bool OnChar(TextInputEvent& e) override;
	bool OnKeyPress(KeyboardEvent& e) override;
	virtual void OnKeyboardFocusIn() override;
	virtual void OnKeyboardFocusOut() override;
private:
	void ReCalculateCursorPixelPosition();
	std::string m_value;
	TextPainter m_painter;
	size_t m_cursor_position;
	int m_cursor_pixel_position{ 1 };
	bool m_focused = false;

	static constexpr int H_Margin = 3;
};
}
