#pragma once

#include "toolgui/Dialog.hpp"
#include "toolgui/LineEditControl.hpp"

namespace node
{

class DialogLabel : public DialogControl
{
public:
	DialogLabel(std::vector<std::string> lines, const WidgetSize& size, TTF_Font* font, Dialog* parent);
	static std::vector<std::string> SplitToLinesofWidth(const std::string& str, TTF_Font* font, int width);
	void OnDraw(SDL::Renderer& renderer) override;
	void SetText(std::vector<std::string> lines);
	static constexpr int LinesMargin = 5;
private:
	std::vector<std::string> m_lines;
	TTF_Font* m_font;
	std::vector<TextPainter> m_painters;
	SDL_Color m_color;
};

class PropertyEditControl : public DialogControl
{
public:
	PropertyEditControl(std::string name, int name_width, std::string initial_value, 
		const WidgetSize& size, TTF_Font* title_font, TTF_Font* error_font, Dialog* parent);
	void OnDraw(SDL::Renderer& renderer) override;
	const std::string& GetValue() const { return m_edit.GetValue(); }
	const std::string& GetName() const { return m_name; }
	void SetErrorText(std::string line);
protected:
	void OnSetSize(const WidgetSize& rect) override;
	MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;
private:
	LineEditControl m_edit;
	std::string m_name;
	TextPainter m_painter;
	TextPainter m_error_painter;
	int m_name_width;

	static constexpr SDL_Color ErrorColor{ 180, 0, 0, 255 };
	static constexpr float EditHeight = 35.0f;
};

class SeparatorControl : public DialogControl
{
public:
	using DialogControl::DialogControl;
	void OnDraw(SDL::Renderer& renderer) override;
};

}