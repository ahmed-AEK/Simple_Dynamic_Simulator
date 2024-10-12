#include "ScopeDisplayDialog.hpp"
#include <array>
#include "SDL_Framework/SDLCPP.hpp"
#include "toolgui/Scene.hpp"
#include "toolgui/Application.hpp"
#include "toolgui/ToolBar.hpp"
#include <charconv>
#include <cstring>
#include "boost/container/static_vector.hpp"

namespace node
{

class ScopeDisplayToolButton : public ToolBarButton
{
public:
	ScopeDisplayToolButton(const SDL_Rect& rect, Scene* parent,
		std::string name, ScopeDisplayToolsManager& manager)
		:ToolBarButton(rect, parent, std::move(name)), m_manager{ manager } {}
	void OnButonClicked() override;
	void SetActive(bool value = true) { b_active = value; }
protected:
	bool IsDisabled() override { return b_active; }
private:
	ScopeDisplayToolsManager& m_manager;
	bool b_active = false;

};


class ScopeArrowTool : public ScopeDisplayTool
{
public:
	explicit ScopeArrowTool(PlotWidget& widget) :ScopeDisplayTool{ "A", widget } {}
private:
};

class ScopeMoveTool : public ScopeDisplayTool
{
public:
	explicit ScopeMoveTool(PlotWidget& widget) :ScopeDisplayTool{ "M", widget } {}
protected:
	MI::ClickEvent OnLMBDown(const SDL_Point& point) override
	{
		if (auto widget = GetWidget())
		{
			auto&& extent = widget->GetExtent();
			m_drag_start_edge = SDL_FPoint{ extent.x, extent.y };
			m_drag_start_point = point;
			b_held_down = true;
		}
		return MI::ClickEvent::CAPTURE_START;
	}

	void OnMouseMove(const SDL_Point& point) override
	{
		if (!b_held_down)
		{
			return;
		}

		if (auto widget = GetWidget())
		{
			auto&& extent = widget->GetExtent();
			auto&& drawing_rect = widget->GetDrawingRect();
			SDL_Point move_distance{ point.x - m_drag_start_point.x, point.y - m_drag_start_point.y };
			SDL_FPoint move_distance_space{ static_cast<float > (move_distance.x) / static_cast<float>(drawing_rect.w) * extent.w,
				static_cast<float>(move_distance.y) / static_cast<float>(drawing_rect.h) * extent.h };
			widget->SetExtend(SDL_FRect{ m_drag_start_edge.x - move_distance_space.x, m_drag_start_edge.y + move_distance_space.y, extent.w, extent.h });
		}
	}

	MI::ClickEvent OnLMBUp(const SDL_Point& point) override
	{
		UNUSED_PARAM(point);
		b_held_down = false;
		return MI::ClickEvent::CAPTURE_END;
	}

	void OnDeactivate() override 
	{
		b_held_down = false;
	}


private:
	SDL_Point m_drag_start_point{ 0,0 };
	SDL_FPoint m_drag_start_edge{ 0.0,0.0 };
	bool b_held_down = false;

};

class ScopeZoomTool : public ScopeDisplayTool
{
public:
	explicit ScopeZoomTool(PlotWidget& widget) :ScopeDisplayTool{ "Z", widget } {}
protected:
	MI::ClickEvent OnLMBDown(const SDL_Point& point) override
	{
		if (auto widget = GetWidget())
		{
			SDL_Rect inner_rect = widget->GetDrawingRect();
			if (SDL_PointInRect(&point, &inner_rect))
			{
				m_drag_start_point = point;
				m_current_drag_point = point;
				b_held_down = true;
			}
		}
		return MI::ClickEvent::CAPTURE_START;
	}

	void OnMouseMove(const SDL_Point& point) override
	{
		if (!b_held_down)
		{
			return;
		}

		if (auto widget = GetWidget())
		{
			SDL_Point contained_point = point;
			const auto& inner_rect = widget->GetDrawingRect();
			contained_point.x = std::clamp(contained_point.x, inner_rect.x, inner_rect.x + inner_rect.w);
			contained_point.y = std::clamp(contained_point.y, inner_rect.y, inner_rect.y + inner_rect.h);
			m_current_drag_point = contained_point;
		}
	}

	MI::ClickEvent OnLMBUp(const SDL_Point& point) override
	{
		if (!b_held_down)
		{
			return MI::ClickEvent::NONE;
		}


		if (auto widget = GetWidget())
		{
			SDL_Point contained_point = point;
			const auto& inner_rect = widget->GetDrawingRect();
			contained_point.x = std::clamp(contained_point.x, inner_rect.x, inner_rect.x + inner_rect.w);
			contained_point.y = std::clamp(contained_point.y, inner_rect.y, inner_rect.y + inner_rect.h);
			const auto& extent = widget->GetExtent();
			auto ScreenToSpace = [&](const SDL_Point& p) -> SDL_FPoint
				{
					return { static_cast<float>(p.x - inner_rect.x) / (inner_rect.w) * extent.w + extent.x,
						extent.y + extent.h - static_cast<float>(p.y - inner_rect.y) / inner_rect.h * extent.h };
				};
			auto first_point = ScreenToSpace(m_drag_start_point);
			auto second_point = ScreenToSpace(contained_point);
			auto start_point = SDL_FPoint{std::min(first_point.x, second_point.x), std::min(first_point.y, second_point.y)};
			auto end_point = SDL_FPoint{ std::max(first_point.x, second_point.x), std::max(first_point.y, second_point.y) };

			widget->SetExtend(SDL_FRect{ start_point.x, start_point.y, end_point.x - start_point.x, end_point.y - start_point.y });
		}
		UNUSED_PARAM(point);
		b_held_down = false;
		return MI::ClickEvent::CAPTURE_END;
	}

	void OnDeactivate() override
	{
		b_held_down = false;
	}

	void Draw(SDL_Renderer* renderer) override
	{
		if (b_held_down)
		{
			auto start_point = SDL_Point{ std::min(m_drag_start_point.x, m_current_drag_point.x), std::min(m_drag_start_point.y, m_current_drag_point.y) };
			auto end_point = SDL_Point{ std::max(m_drag_start_point.x, m_current_drag_point.x), std::max(m_drag_start_point.y, m_current_drag_point.y) };
			auto draw_rect = SDL_Rect{ start_point.x, start_point.y, end_point.x - start_point.x, end_point.y - start_point.y };
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderDrawRect(renderer, &draw_rect);
		}
	}

private:
	bool b_held_down = false;
	SDL_Point m_drag_start_point{ 0,0 };
	SDL_Point m_current_drag_point{ 0,0 };
};

}

node::ScopeDisplayTool::ScopeDisplayTool(std::string name, PlotWidget& widget)
	: m_name{ std::move(name) }, m_plot_widget{widget.GetMIHandlePtr()}
{
}

node::PlotWidget* node::ScopeDisplayTool::GetWidget()
{
	return static_cast<PlotWidget*>(m_plot_widget.GetObjectPtr());
}

void node::ScopeDisplayToolsManager::ChangeTool(std::string_view tool_name)
{
	std::shared_ptr<ScopeDisplayTool> next_tool;
	for (auto&& tool : m_tools)
	{
		if (tool->GetName() == tool_name)
		{
			next_tool = tool;
			break;
		}
	}

	if (!next_tool)
	{
		return;
	}

	if (auto ptr = m_plot_widget.GetObjectPtr())
	{
		static_cast<PlotWidget*>(ptr)->SetTool(next_tool);
	}

	for (auto&& btn : m_buttons)
	{
		if (auto ptr = btn.GetObjectPtr())
		{
			if (static_cast<ScopeDisplayToolButton*>(ptr)->GetName() == tool_name)
			{
				static_cast<ScopeDisplayToolButton*>(ptr)->SetActive(true);
			}
			else
			{
				static_cast<ScopeDisplayToolButton*>(ptr)->SetActive(false);
			}
		}
	}
}

void node::ScopeDisplayToolsManager::AddTool(std::shared_ptr<ScopeDisplayTool> tool)
{
	assert(tool);
	m_tools.push_back(std::move(tool));
}

void node::ScopeDisplayToolsManager::AddButton(ScopeDisplayToolButton& btn)
{
	m_buttons.push_back(btn.GetMIHandlePtr());
}

void node::ScopeDisplayToolsManager::SetWidget(PlotWidget& widget)
{
	m_plot_widget = widget.GetMIHandlePtr();
}

void node::ScopeDisplayToolButton::OnButonClicked()
{
	m_manager.ChangeTool(m_name);
}

node::ScopeDiplayDialog::ScopeDiplayDialog(const SDL_Rect& rect, Scene* parent)
	:BlockDialog{"Scope Display", rect, parent}
{	
	auto plot = std::make_unique<PlotWidget>(parent->GetApp()->getFont(FontType::Label).get(), SDL_Rect{0,0,400,200}, parent);
	m_tools_manager.SetWidget(*plot);
	{
		SetResizeable(true);

		auto toolbar = std::make_unique<ToolBar>(rect, parent);
		
		toolbar->AddButton(std::make_unique<ToolBarCommandButton>(SDL_Rect{ 0,0,ToolBarButton::width, ToolBarButton::height }, parent,
			"R", [plot_ptr = plot.get()]() {plot_ptr->ResetZoom(); }));
		toolbar->AddSeparator();

		auto A_btn = std::make_unique<ScopeDisplayToolButton>(SDL_Rect{0,0, ToolBarButton::width, ToolBarButton::height}, parent, "A", m_tools_manager);
		auto M_btn = std::make_unique<ScopeDisplayToolButton>(SDL_Rect{ 0,0, ToolBarButton::width, ToolBarButton::height }, parent, "M", m_tools_manager);
		auto Z_btn = std::make_unique<ScopeDisplayToolButton>(SDL_Rect{ 0,0, ToolBarButton::width, ToolBarButton::height }, parent, "Z", m_tools_manager);

		m_tools_manager.AddButton(*A_btn);
		m_tools_manager.AddButton(*M_btn);
		m_tools_manager.AddButton(*Z_btn);

		toolbar->AddButton(std::move(A_btn));
		toolbar->AddButton(std::move(M_btn));
		toolbar->AddButton(std::move(Z_btn));

		m_tools_manager.AddTool(std::make_shared<ScopeArrowTool>(*plot));
		m_tools_manager.AddTool(std::make_shared<ScopeMoveTool>(*plot));
		m_tools_manager.AddTool(std::make_shared<ScopeZoomTool>(*plot));

		SetToolbar(std::move(toolbar));
	}
	m_tools_manager.ChangeTool("A");
	XYSeries series{ {{0,0},{4,4}, {5,0}, {6,3},{7,-1}} };
	plot->SetData(std::move(series));
	plot_widget = plot.get();
	AddControl(std::move(plot));
}

void node::ScopeDiplayDialog::SetData(XYSeries data)
{
	plot_widget->SetData(std::move(data));
}

void node::ScopeDiplayDialog::UpdateResults(std::any new_result)
{
	try
	{
		const auto& data = std::any_cast<const std::vector<std::vector<double>>&>(new_result);
		XYSeries xydata;
		assert(data.size() >= 2);
		const size_t time_index = data.size() - 1;
		for (size_t i = 0; i < data[1].size(); i++)
		{
			xydata.points.push_back(SDL_FPoint{ static_cast<float>(data[time_index][i]), static_cast<float>(data[0][i]) });
		}
		SetData(std::move(xydata));
	}
	catch (std::bad_cast&)
	{
		// do nothing
	}
}

node::PlotWidget::PlotWidget(TTF_Font* font, const SDL_Rect& rect, Scene* parent)
	:DialogControl{rect,parent}, m_font{font}, m_current_point_painter{font}
{
	SetSizingMode(DialogControl::SizingMode::expanding);

	for (int i = 0; i < 18; i++)
	{
		m_painters.emplace_back(font);
	}
}

void node::PlotWidget::Draw(SDL_Renderer* renderer)
{
	DrawAxes(renderer);
	DrawAxesTicks(renderer);
	DrawData(renderer);
	DrawCoords(renderer);
	if (m_tool)
	{
		m_tool->Draw(renderer);
	}

}

void node::PlotWidget::SetData(XYSeries data)
{
	m_data = std::move(data);
	if (!m_data.points.size())
	{
		return;
	}

	SDL_FPoint min_point = m_data.points[0];
	SDL_FPoint max_point = m_data.points[0];
	for (const auto& point : m_data.points)
	{
		min_point.x = std::min(min_point.x, point.x);
		min_point.y = std::min(min_point.y, point.y);
		max_point.x = std::max(max_point.x, point.x);
		max_point.y = std::max(max_point.y, point.y);
	}

	float x_distance = max_point.x - min_point.x;
	float y_distance = max_point.y - min_point.y;
	if (y_distance == 0)
	{
		min_point.y = std::min<float>(min_point.y, 0);
		max_point.y = std::max<float>(max_point.y, 0);
		y_distance = max_point.y - min_point.y;
	}
	min_point.x -= x_distance * 0.1f;
	min_point.y -= y_distance * 0.1f;
	max_point.x += x_distance * 0.1f;
	max_point.y += y_distance * 0.1f;

	
	m_base_space_extent = SDL_FRect{ min_point.x, min_point.y, max_point.x - min_point.x, max_point.y - min_point.y };
	ResetZoom();
	ResetPainters();
	m_data_texture.DropTexture();
}

void node::PlotWidget::ResetZoom()
{
	m_space_extent = m_base_space_extent;
	ResetPainters();
}

void node::PlotWidget::SetTool(std::shared_ptr<ScopeDisplayTool> tool)
{
	if (m_tool && m_tool != tool)
	{
		m_tool->Deactivate();
	}
	m_tool = std::move(tool);
	m_tool->Activate();
}

const SDL_FRect& node::PlotWidget::GetExtent()
{
	return m_space_extent;
}

void node::PlotWidget::SetExtend(const SDL_FRect& rect)
{
	m_space_extent = rect;
	ResetPainters();
}

SDL_Rect node::PlotWidget::GetDrawingRect()
{
	return GetInnerRect();
}

void node::PlotWidget::OnMouseMove(const SDL_Point& current_mouse_point)
{
	SDL_Rect inner_rect = GetInnerRect();
	if (SDL_PointInRect(&current_mouse_point, &inner_rect))
	{
		auto&& extent = GetExtent();
		auto ScreenToSpace = [&](const SDL_Point& p) -> SDL_FPoint
			{
				return { static_cast<float>(p.x - inner_rect.x) / (inner_rect.w) * extent.w + extent.x,
					extent.y + extent.h - static_cast<float>(p.y - inner_rect.y) / inner_rect.h * extent.h };
			};
		m_current_point = ScreenToSpace(current_mouse_point);
		m_current_point_painter.SetText("(" + std::to_string(m_current_point->x) + " , " + std::to_string(m_current_point->y) + ")");
	}
	else
	{
		m_current_point = std::nullopt;
	}

	if (m_tool)
	{
		m_tool->MouseMove(current_mouse_point);
	}
	else
	{
		DialogControl::OnMouseMove(current_mouse_point);
	}
}

MI::ClickEvent node::PlotWidget::OnLMBDown(const SDL_Point& current_mouse_point)
{
	if (m_tool)
	{
		return m_tool->LMBDown(current_mouse_point);
	}
	return DialogControl::OnLMBDown(current_mouse_point);
}

MI::ClickEvent node::PlotWidget::OnLMBUp(const SDL_Point& current_mouse_point)
{
	if (m_tool)
	{
		return m_tool->LMBUp(current_mouse_point);
	}
	return DialogControl::OnLMBUp(current_mouse_point);
}

void node::PlotWidget::OnMouseOut()
{
	m_current_point = std::nullopt;
}

void node::PlotWidget::DrawAxes(SDL_Renderer* renderer)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_Rect inner_rect = GetInnerRect();
	SDL_Rect axes[]{ 
		{ inner_rect.x, inner_rect.y + inner_rect.h, inner_rect.w + 1, 1 },
		{ inner_rect.x, inner_rect.y, 1, inner_rect.h + 1 },
		{ inner_rect.x, inner_rect.y, inner_rect.w + 1, 1 },
		{ inner_rect.x + inner_rect.w, inner_rect.y, 1, inner_rect.h + 1} };
	SDL_RenderFillRects(renderer, axes, static_cast<int>(std::size(axes)));
}

SDL_Rect node::PlotWidget::GetInnerRect()
{
	const int left_margin = 45;
	const int right_margin = 20;
	const int top_margin = 20;
	const int bottom_margin = 40;
	const auto& base_rect = GetRect();
	SDL_Rect inner_rect{ base_rect.x + left_margin, base_rect.y + top_margin, base_rect.w - left_margin - right_margin, base_rect.h - top_margin - bottom_margin };
	return inner_rect;
}

void node::PlotWidget::ResetPainters()
{
	SDL_Rect inner_rect = GetInnerRect();
	m_painters.clear();
	m_data_texture.DropTexture();

	//bool draw_text = m_space_extent.w > std::abs(m_space_extent.x * 0.1) && m_space_extent.h > std::abs(m_space_extent.y * 0.1);
	bool draw_text = true;
	{
		const int spacing = (inner_rect.h) / static_cast<int>(y_ticks_count + 1);
		int y = inner_rect.y + spacing;
		for (size_t i = 0; i < y_ticks_count; i++)
		{
			if (draw_text)
			{
				float tick_value = m_space_extent.y + m_space_extent.h - (static_cast<float>(y) - inner_rect.y) / static_cast<float>(inner_rect.h) * m_space_extent.h;
				char buffer[16]{};
				auto [ptr, ec] = std::to_chars(&buffer[0], buffer + std::size(buffer), tick_value, std::chars_format::fixed, 2);
				if (ec != std::errc{})
				{
					std::memset(buffer, 0, std::size(buffer));
				}
				m_painters.emplace_back(m_font);
				m_painters.back().SetText(std::string{ buffer });
			}
			y += spacing;
		}
	}

	{
		const int spacing = (inner_rect.w) / static_cast<int>(x_ticks_count + 1);
		int x = inner_rect.x + spacing;
		for (size_t i = 0; i < x_ticks_count; i++)
		{
			if (draw_text)
			{
				float tick_value = m_space_extent.x + (static_cast<float>(x) - inner_rect.x) / static_cast<float>(inner_rect.w) * m_space_extent.w;
				char buffer[16]{};
				auto [ptr, ec] = std::to_chars(&buffer[0], buffer + std::size(buffer), tick_value, std::chars_format::fixed, 2);
				if (ec != std::errc{})
				{
					std::memset(buffer, 0, std::size(buffer));
				}
				m_painters.emplace_back(m_font);
				m_painters.back().SetText(std::string{ buffer });
			}
			x += spacing;
		}
	}
}

static float ipart(float x)
{
	return std::floor(x);
}
static float fpart(float x)
{
	return x - ipart(x);
}
static float rfpart(float x)
{
	return 1 - fpart(x);
}

static void drawPoint(float x, float y, float c, const SDL_Color& color, SDL_Surface* surface)
{
	if (x < 0 || x >= surface->w)
	{
		return;
	}
	if (y < 0 || y >= surface->h)
	{
		return;
	}

	int32_t* buffer = static_cast<int32_t*>(surface->pixels);
	unsigned char* data = reinterpret_cast<unsigned char*>(&buffer[static_cast<int>(x) + static_cast<int>(y) * surface->pitch / 4]);
	unsigned char old_alpha = data[3];
	unsigned char new_alpha = std::max(old_alpha, static_cast<unsigned char>(c * 255.9));
	data[0] = color.r;
	data[1] = color.g;
	data[2] = color.b;
	data[3] = new_alpha;
}

static void DrawLineAA(SDL_Surface* surface, SDL_FPoint start, SDL_FPoint end, const SDL_Color& color)
{
	bool steep = std::abs(end.y - start.y) > std::abs(end.x - start.x);

	if (steep)
	{
		//std::swap(start, end);
		std::swap(start.x, start.y);
		std::swap(end.x, end.y);
	}
	if (start.x > end.x)
	{
		std::swap(start, end);
	}

	float dx = end.x - start.x;
	float dy = end.y - start.y;
	float gradient = 1;
	if (dx != 0.)
	{
		gradient = dy / dx;
	}

	// handle first endpoint
	float xend = std::round(start.x);
	float yend = start.y + gradient * (xend - start.x);
	float xgap = rfpart(start.x + 0.5f);
	float ygap = rfpart(start.y + 0.5f);
	float xpxl1 = xend;
	float ypxl1 = ipart(yend);
	if (steep)
	{
		drawPoint(ypxl1 - 1, xpxl1    , rfpart(yend) * xgap, color, surface);
		drawPoint(ypxl1    , xpxl1 - 1, rfpart(xend) * ygap, color, surface);
		drawPoint(ypxl1    , xpxl1    , 1.0f, color, surface);
		drawPoint(ypxl1    , xpxl1 + 1, fpart(xend) * ygap, color, surface);
		drawPoint(ypxl1 + 1, xpxl1    , fpart(yend) * xgap, color, surface);
	}
	else
	{
		drawPoint(xpxl1    , ypxl1 - 1, rfpart(yend) * xgap, color, surface);
		drawPoint(xpxl1 - 1, ypxl1    , rfpart(xend) * ygap, color, surface);
		drawPoint(xpxl1    , ypxl1    , 1.0f, color, surface);
		drawPoint(xpxl1 + 1, ypxl1    , fpart(xend) * ygap, color, surface);
		drawPoint(xpxl1    , ypxl1 + 1, fpart(yend) * xgap, color, surface);
	}

	float intery = yend + gradient;

	// handle second endpoint
	xend = std::round(end.x);
	yend = end.y + gradient * (xend - end.x);
	xgap = fpart(end.x + 0.5f);
	ygap = fpart(end.y + 0.5f);
	float xpxl2 = xend;
	float ypxl2 = ipart(yend);
	if (steep)
	{
		drawPoint(ypxl2 - 1, xpxl2    , rfpart(yend) * xgap, color, surface);
		drawPoint(ypxl2    , xpxl2 - 1, rfpart(xend) * ygap, color, surface);
		drawPoint(ypxl2    , xpxl2    , 1.0f, color, surface);
		drawPoint(ypxl2    , xpxl2 + 1, fpart(xend) * ygap, color, surface);
		drawPoint(ypxl2 + 1, xpxl2    , fpart(yend) * xgap, color, surface);
	}
	else
	{
		drawPoint(xpxl2    , ypxl2 - 1, rfpart(yend) * xgap, color, surface);
		drawPoint(xpxl2 - 1, ypxl2    , rfpart(xend) * ygap, color, surface);
		drawPoint(xpxl2    , ypxl2    , 1.0f, color, surface);
		drawPoint(xpxl2 + 1, ypxl2    , fpart(xend) * ygap, color, surface);
		drawPoint(xpxl2    , ypxl2 + 1, fpart(yend) * xgap, color, surface);
	}

	if (steep)
	{
		for (int x = static_cast<int>(xpxl1 + 1); x < xpxl2; x++)
		{
			drawPoint(ipart(intery) - 1, static_cast<float>(x), rfpart(intery), color, surface);
			drawPoint(ipart(intery)    , static_cast<float>(x), 1.0f, color, surface);
			drawPoint(ipart(intery) + 1, static_cast<float>(x), fpart(intery), color, surface);
			intery = intery + gradient;
		}
	}
	else
	{
		for (int x = static_cast<int>(xpxl1 + 1); x < xpxl2; x++)
		{
			drawPoint(static_cast<float>(x), ipart(intery) - 1, rfpart(intery), color, surface);
			drawPoint(static_cast<float>(x), ipart(intery)    , 1.0f, color, surface);
			drawPoint(static_cast<float>(x), ipart(intery) + 1, fpart(intery), color, surface);
			intery = intery + gradient;
		}
	}
}


static constexpr std::optional<std::array<SDL_FPoint, 2>> clipPointsToScreen(const SDL_FPoint& p1, const SDL_FPoint& p2, const SDL_Rect& extents)
{
	enum class PointSide
	{
		inside,
		left,
		right,
		top,
		bottom,
	};

	auto SideFinder = [&](const SDL_FPoint& p) -> PointSide
		{
			if (p.x < extents.x)
			{
				return PointSide::left;
			}
			if (p.x > extents.x + extents.w)
			{
				return PointSide::right;
			}
			if (p.y < extents.y)
			{
				return PointSide::top;
			}
			if (p.y > extents.y + extents.h)
			{
				return PointSide::bottom;
			}
			return PointSide::inside;
		};
	auto p1_side = SideFinder(p1);
	auto p2_side = SideFinder(p2);

	if (p1_side != PointSide::inside && p1_side == p2_side)
	{
		return std::nullopt;
	}
	if (p1_side == PointSide::inside && p2_side == PointSide::inside)
	{
		return std::optional<std::array<SDL_FPoint, 2>>({ p1, p2 });
	}
	std::array<SDL_FPoint, 2> returned_points{ p1, p2 };
	double dydx = (p2.y - p1.y) / (p2.x - p1.x);
	double dxdy = (p2.x - p1.x) / (p2.y - p1.y);
	auto clipper = [&](const auto& p, auto& target)
		{
			boost::container::static_vector<std::pair<SDL_FPoint, double>,4> intersections;
			// intersect y = 0
			{
				double x_intercept = p.x + dxdy * (-p.y);
				if (x_intercept > 0 && x_intercept < extents.w)
				{
					double distance = (p.x - x_intercept) * (p.x - x_intercept) + p.y * p.y;
					intersections.push_back({ SDL_FPoint{ static_cast<float>(x_intercept), 0 }, distance });
				}
			}
			// intersect y = h
			{
				double x_intercept = p.x + dxdy * (extents.h - p.y);
				if (x_intercept > 0 && x_intercept < extents.w)
				{
					double distance = (p.x - x_intercept) * (p.x - x_intercept) + (p.y - extents.h) * (p.y - extents.h);
					intersections.push_back({ SDL_FPoint{ static_cast<float>(x_intercept), static_cast<float>(extents.h) }, distance });
				}
			}
			// intersect x = 0
			{
				double y_intercept = p.y + dydx * (-p.x);
				if (y_intercept > 0 && y_intercept < extents.h)
				{
					double distance = (p.x) * (p.x) + (p.y - y_intercept) * (p.y - y_intercept);
					intersections.push_back({ SDL_FPoint{ 0, static_cast<float>(y_intercept) }, distance });
				}
			}
			// intersect x = w
			{
				double y_intercept = p.y + dydx * (extents.w - p.x);
				if (y_intercept > 0 && y_intercept < extents.h)
				{
					double distance = (p.x - extents.w) * (p.x - extents.w) + (p.y - y_intercept) * (p.y - y_intercept);
					intersections.push_back({ SDL_FPoint{ static_cast<float>(extents.w), static_cast<float>(y_intercept) }, distance });
				}
			}

			if (intersections.size())
			{
				// return closest point
				std::sort(intersections.begin(), intersections.end(), [](const auto& px, const auto& py) {return std::get<1>(px) < std::get<1>(py); });
				target = std::get<0>(intersections[0]);
			}
		};
	if (p1_side != PointSide::inside)
	{
		clipper(p1, returned_points[0]);
	}
	if (p2_side != PointSide::inside)
	{
		clipper(p2, returned_points[1]);
	}
	return returned_points;	
}

void node::PlotWidget::ReDrawSurface()
{
	std::vector<SDL_FPoint> points;
	points.reserve(m_data.points.size());
	SDL_Rect inner_rect = GetInnerRect();
	m_data_surface.reset(SDL_CreateRGBSurfaceWithFormat(0, inner_rect.w, inner_rect.h, 32, SDL_PIXELFORMAT_RGBA32));
	for (const auto& point : m_data.points)
	{
		SDL_FPoint transformed{ (point.x - m_space_extent.x) / (m_space_extent.w) * inner_rect.w, inner_rect.h - (point.y - m_space_extent.y) / (m_space_extent.h) * inner_rect.h };
		points.push_back({ transformed.x, transformed.y });
	}
	if (points.size() < 2)
	{
		return;
	}
	SDL_Color color{ 30,120,180,255 };
	SDL_Rect surface_extents{ 0,0,m_data_surface->w, m_data_surface->h };
	for (size_t i = 1; i < points.size(); i++)
	{
		auto line_extents = clipPointsToScreen(points[i - 1], points[i], surface_extents);
		if (line_extents)
		{
			DrawLineAA(m_data_surface.get(), (*line_extents)[0], (*line_extents)[1], color);
		}
	}
}

void node::PlotWidget::DrawData(SDL_Renderer* renderer)
{
	int tex_w, tex_h;
	SDL_QueryTexture(m_data_texture.GetTexture(), nullptr, nullptr, &tex_w, &tex_h);
	SDL_Rect inner_rect = GetInnerRect();

	if (!m_data_texture || inner_rect.w != tex_w || inner_rect.h != tex_h)
	{
		ReDrawSurface();
		m_data_texture.SetTexture(SDLTexture{ SDL_CreateTextureFromSurface(renderer, m_data_surface.get()) });
	}
	SDL_RenderCopy(renderer, m_data_texture.GetTexture(), nullptr, &inner_rect);
}

void node::PlotWidget::DrawAxesTicks(SDL_Renderer* renderer)
{
	SDL_Rect inner_rect = GetInnerRect();
	const int tick_length = 5;
	size_t painter_idx = 0;
	//bool draw_text = m_space_extent.w > std::abs(m_space_extent.x * 0.1) && m_space_extent.h > std::abs(m_space_extent.y * 0.1);
	bool draw_text = true;
	{
		std::array<SDL_Rect, y_ticks_count> left_ticks;
		{
			const int spacing = (inner_rect.h) / static_cast<int>(left_ticks.size() + 1);
			int y = inner_rect.y + spacing;
			for (size_t i = 0; i < left_ticks.size(); i++)
			{
				if (draw_text)
				{
					SDL_Color Black = { 50, 50, 50, 255 };
					SDL_Rect text_rect = m_painters[painter_idx].GetRect(renderer, Black);
					text_rect.x = inner_rect.x - 5 - tick_length - text_rect.w;
					text_rect.y = y - text_rect.h / 2;
					m_painters[painter_idx].Draw(renderer, { text_rect.x, text_rect.y }, Black);
					painter_idx += 1;
				}

				left_ticks[i] = SDL_Rect{ inner_rect.x - tick_length, y, tick_length + 1, 1 };
				y += spacing;
			}
		}
		SDL_RenderFillRects(renderer, left_ticks.data(), static_cast<int>(std::size(left_ticks)));
	}

	{
		std::array<SDL_Rect, x_ticks_count> bottom_ticks;
		{
			const int spacing = (inner_rect.w) / static_cast<int>(bottom_ticks.size() + 1);
			int x = inner_rect.x + spacing;
			for (size_t i = 0; i < bottom_ticks.size(); i++)
			{
				if (draw_text)
				{
					SDL_Color Black = { 50, 50, 50, 255 };
					
					SDL_Rect text_rect = m_painters[painter_idx].GetRect(renderer, Black);
					text_rect.x = x - text_rect.w/2;
					text_rect.y = inner_rect.y + inner_rect.h + tick_length + 5;
					m_painters[painter_idx].Draw(renderer, { text_rect.x, text_rect.y }, Black);
					painter_idx += 1;
				}
				

				bottom_ticks[i] = SDL_Rect{ x, inner_rect.y + inner_rect.h, 1, tick_length + 1 };
				x += spacing;
			}
		}
		SDL_RenderFillRects(renderer, bottom_ticks.data(), static_cast<int>(std::size(bottom_ticks)));
	}
}

void node::PlotWidget::DrawCoords(SDL_Renderer* renderer)
{
	if (m_current_point)
	{
		SDL_Color black{ 50, 50, 50, 255};
		auto&& rect = GetInnerRect();
		auto text_rect = m_current_point_painter.GetRect(renderer, black);
		SDL_Point start_point{ rect.x + rect.w - text_rect.w, rect.y - text_rect.h - 2};
		m_current_point_painter.Draw(renderer, start_point, black);
	}
}
