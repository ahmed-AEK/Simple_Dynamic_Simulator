#include "ScopeDisplayDialog.hpp"
#include <array>
#include "SDL_Framework/SDLCPP.hpp"
#include "toolgui/Scene.hpp"
#include "toolgui/Application.hpp"

node::ScopeDiplayDialog::ScopeDiplayDialog(const SDL_Rect& rect, Scene* parent)
	:Dialog{"Scope Display", rect, parent}
{
	auto plot = std::make_unique<PlotWidget>(parent->GetApp()->getFont(FontType::Label).get(), SDL_Rect{0,0,500,400}, parent);
	XYSeries series{ {{0,0},{1,1},{2,2},{3,3},{4,4}, {5,0}, {6,3},{7,-1}} };
	plot->SetData(std::move(series));
	AddControl(std::move(plot));
}

node::PlotWidget::PlotWidget(TTF_Font* font, const SDL_Rect& rect, Scene* parent)
	:DialogControl{rect,parent}, m_font{font}
{
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
	const float x_distance = max_point.x - min_point.x;
	const float y_distance = max_point.y - min_point.y;
	min_point.x -= x_distance * 0.1f;
	min_point.y -= y_distance * 0.1f;
	max_point.x += x_distance * 0.1f;
	max_point.y += y_distance * 0.1f;
	m_space_extent = SDL_FRect{ min_point.x, min_point.y, max_point.x - min_point.x, max_point.y - min_point.y };

	ResetPainters();

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
	
	bool draw_text = m_space_extent.w > std::abs(m_space_extent.x * 0.1) && m_space_extent.h > std::abs(m_space_extent.y * 0.1);
	{
		{
			const int spacing = (inner_rect.h) / static_cast<int>(y_ticks_count + 1);
			int y = inner_rect.y + spacing;
			for (size_t i = 0; i < y_ticks_count; i++)
			{
				if (draw_text)
				{
					float tick_value = m_space_extent.y + m_space_extent.h - (static_cast<float>(y) - inner_rect.y) / static_cast<float>(inner_rect.h) * m_space_extent.h;
					std::string tick_text = std::to_string(tick_value);
					size_t allowed_chars = 4;
					if (tick_text.size() && tick_text[0] == '-')
					{
						allowed_chars += 1;
					}
					while (tick_text.size() > allowed_chars)
					{
						tick_text.pop_back();
					}
					m_painters.emplace_back(m_font);
					m_painters.back().SetText(tick_text);
				}
				y += spacing;
			}
		}
	}

	{
		{
			const int spacing = (inner_rect.w) / static_cast<int>(x_ticks_count + 1);
			int x = inner_rect.x + spacing;
			for (size_t i = 0; i < x_ticks_count; i++)
			{
				if (draw_text)
				{
					float tick_value = m_space_extent.x + (static_cast<float>(x) - inner_rect.x) / static_cast<float>(inner_rect.w) * m_space_extent.w;
					std::string tick_text = std::to_string(tick_value);
					size_t allowed_chars = 4;
					if (tick_text.size() && tick_text[0] == '-')
					{
						allowed_chars += 1;
					}
					while (tick_text.size() > allowed_chars)
					{
						tick_text.pop_back();
					}
					m_painters.emplace_back(m_font);
					m_painters.back().SetText(tick_text);
				}
				x += spacing;
			}
		}
	}
}

void node::PlotWidget::DrawData(SDL_Renderer* renderer)
{
	std::vector<SDL_Point> points;
	points.reserve(m_data.points.size());
	SDL_Rect inner_rect = GetInnerRect();
	for (const auto& point : m_data.points)
	{
		SDL_FPoint transformed{ (point.x - m_space_extent.x) / (m_space_extent.w) * inner_rect.w + inner_rect.x, inner_rect.y + inner_rect.h - (point.y - m_space_extent.y) / (m_space_extent.h) * inner_rect.h};
		points.push_back({ static_cast<int>(transformed.x), static_cast<int>(transformed.y) });
	}
	SDL_SetRenderDrawColor(renderer, 30, 120, 180, 255);
	SDL_RenderDrawLines(renderer, points.data(), static_cast<int>(points.size()));
	for (auto&& point : points)
	{
		point.y += 1;
	}
	SDL_RenderDrawLines(renderer, points.data(), static_cast<int>(points.size()));

}

void node::PlotWidget::DrawAxesTicks(SDL_Renderer* renderer)
{
	SDL_Rect inner_rect = GetInnerRect();
	const int tick_length = 5;
	size_t painter_idx = 0;
	bool draw_text = m_space_extent.w > std::abs(m_space_extent.x * 0.1) && m_space_extent.h > std::abs(m_space_extent.y * 0.1);
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
