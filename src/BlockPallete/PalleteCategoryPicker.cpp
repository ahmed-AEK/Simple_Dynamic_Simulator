#include "PalleteCategoryPicker.hpp"
#include "PalleteProvider.hpp"

node::PalleteCategoryPicker::PalleteCategoryPicker(const SDL_FRect& rect, 
	std::shared_ptr<PalleteProvider> provider, TTF_Font* font, Widget* parent)
	:Widget{rect, parent}, m_palleteProvider{provider}, m_title_painter{font}
{
	m_title_painter.SetText("Block Pallete");
	assert(font);
	if (m_palleteProvider)
	{
		m_palleteProvider->Attach(*this);
		ResetCategories();
	}
}

void node::PalleteCategoryPicker::SetProvider(std::shared_ptr<PalleteProvider> provider)
{
	if (m_palleteProvider)
	{
		m_palleteProvider->Detach(*this);
	}
	m_palleteProvider = std::move(provider);
	if (m_palleteProvider)
	{
		m_palleteProvider->Attach(*this);
		ResetCategories();
	}
}

void node::PalleteCategoryPicker::OnNotify(BlockPalleteChange& e)
{
	UNUSED_PARAM(e);
	ResetCategories();
}

MI::ClickEvent node::PalleteCategoryPicker::OnLMBDown(MouseButtonEvent& e)
{
	auto&& point = e.point();
	if (point.x < m_categories_draw_area.x ||
		point.x > m_categories_draw_area.x + m_categories_draw_area.w)
	{
		return MI::ClickEvent::NONE;
	}
	if (point.y < m_categories_draw_area.y ||
		point.y > m_categories_draw_area.y + m_categories_draw_area.h)
	{
		return MI::ClickEvent::NONE;
	}

	float y_value = m_categories_draw_area.y;
	const float y_pad = 5;
	for (size_t i = 0; i < m_categories.size(); i++)
	{
		float y_max = y_value + 2 * y_pad + m_categories[i].height;
		if (point.y > y_value && point.y <= y_max)
		{
			Notify({ m_categories[i].painter.GetText() });
			return MI::ClickEvent::CLICKED;
		}
	}
	return MI::ClickEvent::NONE;
}

SDL_FRect node::PalleteCategoryPicker::DrawPanelBorder(SDL_Renderer* renderer)
{
	SDL_FRect draw_area = GetRect();
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(renderer, &draw_area);
	
	SDL_Color Black{ 50,50,50,255 };
	SDL_FRect title_rect = m_title_painter.GetRect(renderer, Black);
	float title_start_x = draw_area.x + draw_area.w / 2 - title_rect.w / 2;
	m_title_painter.Draw(renderer, { title_start_x, draw_area.y + 5 }, Black);

	draw_area.x += 5;
	draw_area.w -= 10;
	draw_area.y += 5 + 5 + title_rect.h;
	draw_area.h -= 10 + 5 + title_rect.h;
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &draw_area);

	draw_area.x += 2;
	draw_area.w -= 4;
	draw_area.y += 2;
	draw_area.h -= 4;
	SDL_FRect inner_rect = draw_area;
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(renderer, &draw_area);
	return inner_rect;
}

void node::PalleteCategoryPicker::ResetCategories()
{
	const auto& labels = m_palleteProvider->GetCategories();

	// make the categories size equal to the labels size
	while (m_categories.size() < labels.size())
	{
		m_categories.emplace_back(TextPainter{ m_title_painter.GetFont() });
	}
	while (m_categories.size() > labels.size())
	{
		m_categories.pop_back();
	}

	for (size_t i = 0; i < labels.size(); i++)
	{
		m_categories[i].painter.SetText(labels[i]);
	}
}

void node::PalleteCategoryPicker::Draw(SDL_Renderer* renderer)
{
	auto&& rect = DrawPanelBorder(renderer);

	float x_value = rect.x + 4;
	const float y_pad = 5;
	float y_value = rect.y + y_pad;
	m_categories_draw_area = rect;

	SDL_Color Black{ 50,50,50,255 };
	for (auto&& category : m_categories)
	{
		auto&& painter = category.painter;
		auto&& text_rect = painter.GetRect(renderer, Black);
		
		category.height = text_rect.h; // needed for click events

		painter.Draw(renderer, { x_value, y_value }, Black);
		float line_y = y_value + y_pad + text_rect.h;
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderLine(renderer, rect.x, line_y, rect.x + rect.w, line_y);
		y_value += text_rect.h + 2 * y_pad;
	}
}
