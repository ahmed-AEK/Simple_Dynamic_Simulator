#pragma once

#include "PluginAPI/BlockDialog.hpp"
#include <array>

namespace node
{

struct XYSeries
{
	std::vector<SDL_FPoint> points;
};

class PlotWidget;

class ScopeDisplayTool
{
public:
	ScopeDisplayTool(std::string name, PlotWidget& widget);
	MI::ClickEvent LMBDown(const SDL_FPoint& point) { return OnLMBDown(point); }
	MI::ClickEvent LMBUp(const SDL_FPoint& point) { return OnLMBUp(point); }
	void MouseMove(const SDL_FPoint& point) { OnMouseMove(point); }
	void Activate() { OnActivate(); }
	void Deactivate() { OnDeactivate(); }
	virtual void Draw(SDL::Renderer& renderer) { UNUSED_PARAM(renderer); }
	void Cancel() { OnCancel(); }
	const std::string& GetName() const { return m_name; }
	PlotWidget* GetWidget();
	virtual ~ScopeDisplayTool() = default;
protected:
	virtual MI::ClickEvent OnLMBDown(const SDL_FPoint& point) { UNUSED_PARAM(point); return MI::ClickEvent::NONE; }
	virtual MI::ClickEvent OnLMBUp(const SDL_FPoint& point) { UNUSED_PARAM(point); return MI::ClickEvent::NONE; }
	virtual void OnMouseMove(const SDL_FPoint& point) { UNUSED_PARAM(point); }
	virtual void OnActivate() { }
	virtual void OnDeactivate() {}
	virtual void OnCancel() {}
private:
	std::string m_name;
	HandlePtrS<PlotWidget, Widget> m_plot_widget;
};

class ScopeDisplayToolButton;

class ScopeDisplayToolsManager
{
public:
	ScopeDisplayToolsManager() {}
	void ChangeTool(std::string_view tool_name);
	void AddTool(std::shared_ptr<ScopeDisplayTool> tool);
	void AddButton(ScopeDisplayToolButton& btn);
	void SetWidget(PlotWidget& widget);
private:
	std::vector<std::shared_ptr<ScopeDisplayTool>> m_tools;
	std::vector<HandlePtrS<ScopeDisplayToolButton, Widget>> m_buttons;
	HandlePtrS<PlotWidget, Widget> m_plot_widget;
};

class PlotWidget : public DialogControl
{
public:
	PlotWidget(TTF_Font* font, const WidgetSize& size, Dialog* parent);
	void SetData(XYSeries data);
	void ResetZoom();
	void SetTool(std::shared_ptr<ScopeDisplayTool> tool);
	const SDL_FRect& GetExtent();
	void SetExtend(const SDL_FRect& rect);
	SDL_Rect GetDrawingRect();
protected:
	void OnDraw(SDL::Renderer& renderer) override;
	void OnMouseMove(MouseHoverEvent& e) override;
	MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;
	MI::ClickEvent OnLMBUp(MouseButtonEvent& e) override;
	void OnMouseOut(MouseHoverEvent& e) override;
private:
	void DrawAxes(SDL::Renderer& renderer);
	void DrawData(SDL::Renderer& renderer);
	void DrawAxesTicks(SDL::Renderer& renderer);
	void DrawCoords(SDL::Renderer& renderer);
	SDL_Rect GetInnerRect();
	void ResetPainters();

	void ReDrawSurface();

	static constexpr size_t x_ticks_count = 9;
	static constexpr size_t y_ticks_count = 9;
	static constexpr float top_margin = 20;
	static constexpr float bottom_margin = 40;
	static constexpr float right_margin = 1;
	static constexpr float text_margin = 5;

	TTF_Font* m_font;
	XYSeries m_data;
	SDL_FRect m_base_space_extent{ 0,0,0,0 };
	SDL_FRect m_space_extent{ 0,0,0,0 };
	SDL_FRect m_inner_rect{100,100,100,100};
	std::array<TextPainter, x_ticks_count> m_x_painters;
	std::array<TextPainter, y_ticks_count> m_y_painters;
	std::shared_ptr<ScopeDisplayTool> m_tool;
	std::optional<SDL_FPoint> m_current_point{ SDL_FPoint{ 0,0 } };
	TextPainter m_current_point_painter;
	DroppableTexture m_data_texture;
	SDLSurface m_data_surface;
};

class ScopeDiplayDialog : public BlockDialog
{
public:
	ScopeDiplayDialog(const WidgetSize& size, Scene* parent);
	void SetData(XYSeries data);
	void UpdateResults(std::any new_result) override;

private:
	PlotWidget* plot_widget = nullptr;
	ScopeDisplayToolsManager m_tools_manager;
};

}