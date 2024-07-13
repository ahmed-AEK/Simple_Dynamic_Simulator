#include "BlockPallete.hpp"
#include <cassert>

namespace PalleteData {
	static constexpr int ElementHeight = 200;
	static constexpr int ElementHPadding = 10;
	static constexpr int ElementTotalHeight = ElementHeight + ElementHPadding;
	static constexpr int ElementWidth = 150;
	static constexpr int ElementWPadding = 10;
	static constexpr int ElementTotalWidth = ElementWidth + ElementWPadding;
	static constexpr int scrollbarWidth = 10;
}

node::BlockPallete::BlockPallete(SDL_Rect rect, 
	std::shared_ptr<PalleteProvider> provider, Scene* parent)
	: m_palleteProvider{provider}, Widget(rect, parent)
{
}

void node::BlockPallete::Draw(SDL_Renderer* renderer)
{
	SDL_Rect inner_area = DrawPanelBorder(renderer);
	SDL_RenderSetClipRect(renderer, &inner_area);
	DrawElements(renderer, inner_area);
	SDL_RenderSetClipRect(renderer, nullptr);
}

bool node::BlockPallete::OnScroll(const double amount, const SDL_Point& p)
{
	if (!m_palleteProvider)
	{
		return false;
	}

	SDL_Rect inner_rect = GetRect();
	inner_rect.x += 7;
	inner_rect.y += 7;
	inner_rect.w -= 14;
	inner_rect.h -= 14;

	if (SDL_PointInRect(&p, &inner_rect))
	{
		int elements_height = static_cast<int>(
			m_palleteProvider->GetElements().size() * PalleteData::ElementTotalHeight);
		m_scrollPos -= static_cast<int>(amount * 10);
		if (m_scrollPos > (elements_height - inner_rect.h))
		{
			m_scrollPos = elements_height - inner_rect.h;
		}
		if (m_scrollPos < 0)
		{
			m_scrollPos = 0;
		}
	}

	return true;
}

SDL_Rect node::BlockPallete::DrawPanelBorder(SDL_Renderer* renderer)
{
	SDL_Rect draw_area = GetRect();
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(renderer, &draw_area);

	draw_area.x += 5;
	draw_area.w -= 10;
	draw_area.y += 5;
	draw_area.h -= 10;
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &draw_area);

	draw_area.x += 2;
	draw_area.w -= 4;
	draw_area.y += 2;
	draw_area.h -= 4;
	SDL_Rect inner_rect = draw_area;
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(renderer, &draw_area);
	inner_rect.w -= PalleteData::scrollbarWidth;
	SDL_Rect scrollbar_rect = { inner_rect.x + inner_rect.w, inner_rect.y,
	PalleteData::scrollbarWidth, inner_rect.h };
	DrawScrollBar(renderer, scrollbar_rect);
	return inner_rect;
}

void node::BlockPallete::DrawScrollBar(SDL_Renderer* renderer, const SDL_Rect& area)
{
	SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
	SDL_RenderFillRect(renderer, &area);
	int total_height = 
		static_cast<int>(m_palleteProvider->GetElements().size() * 
			PalleteData::ElementTotalHeight);
	int view_area_height = area.h;
	if (total_height < view_area_height)
	{
		return;
	}
	int scrollbar_height = (view_area_height * area.h) / total_height + 1;
	int scrollbar_start_pos = (m_scrollPos * view_area_height) / total_height;
	SDL_Rect draw_area{ area.x, area.y + scrollbar_start_pos, area.w, scrollbar_height };
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &draw_area);
}

void node::BlockPallete::DrawElements(SDL_Renderer* renderer, const SDL_Rect& area)
{
	if (!m_palleteProvider)
	{
		return;
	}

	auto&& elements = m_palleteProvider->GetElements();
	int possible_elements_on_screen = area.h / PalleteData::ElementTotalHeight + 2;
	int start_element = m_scrollPos / PalleteData::ElementTotalHeight;
	int max_element = std::min(possible_elements_on_screen + start_element, 
		static_cast<int>(elements.size()));
	for (int i = start_element; i < max_element; i++)
	{
		int padding_left = (area.w - PalleteData::ElementTotalWidth) / 2;
		SDL_Rect element_rect = { area.x + padding_left, area.y + i * PalleteData::ElementTotalHeight - m_scrollPos,
			PalleteData::ElementTotalWidth, PalleteData::ElementTotalHeight };
		if (element_rect.w > area.w)
		{
			element_rect.w = area.w;
		}
		if (element_rect.y + element_rect.h > area.y + area.h)
		{
			element_rect.h = area.y + area.h - element_rect.y;
			if (element_rect.h <= 0)
			{
				continue;
			}
		}
		DrawElement(renderer, *elements[i], element_rect);
	}
}

void node::BlockPallete::DrawElement(SDL_Renderer* renderer, const PalleteElement& element, const SDL_Rect& area)
{
	UNUSED_PARAM(element);
	SDL_Rect contained_rect = { area.x + PalleteData::ElementWPadding/2, 
		area.y + PalleteData::ElementHPadding/2,
		PalleteData::ElementWidth, PalleteData::ElementHeight };
	SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
	SDL_RenderFillRect(renderer, &area);

	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	SDL_RenderFillRect(renderer, &contained_rect);

}
