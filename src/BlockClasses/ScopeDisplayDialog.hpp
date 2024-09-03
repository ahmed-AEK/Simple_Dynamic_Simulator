#pragma once

#include "toolgui/Dialog.hpp"

namespace node
{

struct XYSeries
{
	std::vector<SDL_FPoint> points;
};

class PlotWidget : public DialogControl
{
public:
	PlotWidget(TTF_Font* font, const SDL_Rect& rect, Scene* parent);
	void Draw(SDL_Renderer* renderer) override;
	void SetData(XYSeries data);
private:
	void DrawAxes(SDL_Renderer* renderer);
	void DrawData(SDL_Renderer* renderer);
	void DrawAxesTicks(SDL_Renderer* renderer);
	SDL_Rect GetInnerRect();
	void ResetPainters();

	TTF_Font* m_font;
	XYSeries m_data;
	SDL_FRect m_space_extent{ 0,0,0,0 };
	std::vector<TextPainter> m_painters;

	static constexpr size_t x_ticks_count = 9;
	static constexpr size_t y_ticks_count = 9;
};

class ScopeDiplayDialog : public Dialog
{
public:
	ScopeDiplayDialog(const SDL_Rect& rect, Scene* parent);
private:
	
};

}