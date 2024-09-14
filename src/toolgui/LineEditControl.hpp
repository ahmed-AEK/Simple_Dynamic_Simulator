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
	void OnChar(int32_t key) override;
	void OnKeyPress(int32_t key) override;
private:
	std::string m_value;
	TextPainter m_painter;
};
}
