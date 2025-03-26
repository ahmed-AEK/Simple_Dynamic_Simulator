#pragma once

#include "toolgui/Widget.hpp"
#include "SDL_Framework/Utility.hpp"

namespace node
{
class LineEditControl : public Widget
{
public:
	LineEditControl(std::string initial_value, const WidgetSize& size, TTF_Font* font, Widget* parent);
	const std::string& GetValue() const { return m_value; }
	~LineEditControl() override;
protected:
	void OnDraw(SDL::Renderer& renderer) override;
	MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;
	bool OnChar(TextInputEvent& e) override;
	bool OnKeyPress(KeyboardEvent& e) override;
	virtual void OnKeyboardFocusIn() override;
	virtual void OnKeyboardFocusOut() override;
private:
	void ReCalculateCursorPixelPosition();
	RoundRectPainter m_outer_painter;
	RoundRectPainter m_inner_painter;
	std::string m_value;
	TextPainter m_painter;
	size_t m_cursor_position;
	int m_cursor_pixel_position{ 1 };
	bool m_focused = false;

	static constexpr int H_Margin = 3;
};
}
