#pragma once

#include "toolgui/Dialog.hpp"
#include "toolgui/LineEditControl.hpp"

namespace node
{

class DialogLabel : public DialogControl
{
public:
	DialogLabel(std::vector<std::string> lines, const SDL_Rect& rect, TTF_Font* font, Scene* parent);
	static std::vector<std::string> SplitToLinesofWidth(const std::string& str, TTF_Font* font, int width);
	void Draw(SDL_Renderer* renderer) override;

	static constexpr int LinesMargin = 5;
private:
	std::vector<std::string> m_lines;
	TTF_Font* m_font;
	std::vector<TextPainter> m_painters;
};

class PropertyEditControl : public DialogControl
{
public:
	PropertyEditControl(std::string name, int name_width, std::string initial_value, const SDL_Rect& rect, Scene* parent);
	void Draw(SDL_Renderer* renderer) override;
	const std::string& GetValue() const { return m_edit.GetValue(); }
	const std::string& GetName() const { return m_name; }
protected:
	void OnSetRect(const SDL_Rect& rect) override;
	Widget* OnGetInteractableAtPoint(const SDL_Point& point) override;

private:
	LineEditControl m_edit;
	std::string m_name;
	TextPainter m_painter;
	int m_name_width;
};

class SeparatorControl : public DialogControl
{
public:
	using DialogControl::DialogControl;
	void Draw(SDL_Renderer* renderer) override;
};

}