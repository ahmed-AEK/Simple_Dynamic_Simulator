#include "BlockPallete.hpp"
#include <cassert>
#include "SDL_Framework/SDLCPP.hpp"
#include "toolgui/Scene.hpp"
#include "toolgui/Application.hpp"
#include "NodeSDLStylers/BlockStylerFactory.hpp"

namespace PalleteData {
	static constexpr int TopPadding = 20;
	static constexpr int ElementHPadding = 50;
	static constexpr int ElementTotalHeight = node::BlockPallete::ElementHeight + ElementHPadding;
	static constexpr int ElementWPadding = 10;
	static constexpr int ElementTotalWidth = node::BlockPallete::ElementWidth + ElementWPadding;
	static constexpr int scrollbarWidth = 10;
}

node::BlockPallete::BlockPallete(const SDL_FRect& rect,
	std::shared_ptr<PalleteProvider> provider, Widget* parent)
	: Widget(rect, parent), m_palleteProvider{provider}
{
}

void node::BlockPallete::Draw(SDL_Renderer* renderer)
{
	SDL_FRect inner_area = DrawPanelBorder(renderer);
	SDL_Rect inner_area_int = ToRect(inner_area);
	SDL_SetRenderClipRect(renderer, &inner_area_int);
	DrawElements(renderer, inner_area);
	SDL_SetRenderClipRect(renderer, nullptr);
}

bool node::BlockPallete::OnScroll(const double amount, const SDL_FPoint& p)
{
	if (!m_palleteProvider)
	{
		return false;
	}

	SDL_FRect inner_rect = GetRect();
	inner_rect.x += 7;
	inner_rect.y += 7;
	inner_rect.w -= 14;
	inner_rect.h -= 14;

	if (SDL_PointInRectFloat(&p, &inner_rect))
	{
		float elements_height = static_cast<float>(
			m_palleteProvider->GetElements().size() * PalleteData::ElementTotalHeight + PalleteData::TopPadding);
		m_scrollPos -= static_cast<int>(amount * 30);
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

MI::ClickEvent node::BlockPallete::OnLMBDown(MouseButtonEvent& e)
{
	SDL_FPoint current_mouse_point{ e.point() };
	SDL_FRect inner_area = GetRect();
	inner_area.x += 7;
	inner_area.y += 7;
	inner_area.w -= 14;
	inner_area.h -= 14;
	float Hpad_size = inner_area.w - PalleteData::ElementTotalWidth;
	inner_area.x += Hpad_size / 2;
	inner_area.w -= Hpad_size / 2;

	if (!SDL_PointInRectFloat(&current_mouse_point, &inner_area))
	{
		return MI::ClickEvent::NONE;
	}
	float selected_y = current_mouse_point.y - inner_area.y + m_scrollPos -
		PalleteData::TopPadding;
	int selected_item_index = static_cast<int>(selected_y / PalleteData::ElementTotalHeight);

	auto&& pallete_elements = m_palleteProvider->GetElements();
	assert(selected_item_index >= 0);
	assert(static_cast<size_t>(selected_item_index) < pallete_elements.size());

	auto&& selected_element = pallete_elements[selected_item_index];
	auto block_data_ref = model::BlockDataCRef{ selected_element->block, {}};
	if (auto functional_data = selected_element->data.GetFunctionalData())
	{
		block_data_ref = model::BlockDataCRef{ selected_element->block,
			model::BlockDataCRef::FunctionalRef{*functional_data} 
		};
	}
	
	GetApp()->GetScene()->StartDragObject(
		DragDropObject{ selected_element->block_template,
			model::BlockModel{selected_element->block},
			std::shared_ptr<BlockStyler>{
				m_palleteProvider->GetStylerFactory().GetStyler(selected_element->block.GetStyler(), block_data_ref)
			},
			model::BlockData{selected_element->data}
		});
	
	return MI::ClickEvent::NONE;
}

SDL_FRect node::BlockPallete::DrawPanelBorder(SDL_Renderer* renderer)
{
	SDL_FRect draw_area = GetRect();
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
	SDL_FRect inner_rect = draw_area;
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(renderer, &draw_area);
	inner_rect.w -= PalleteData::scrollbarWidth;
	SDL_FRect scrollbar_rect = { inner_rect.x + inner_rect.w, inner_rect.y,
	PalleteData::scrollbarWidth, inner_rect.h };
	DrawScrollBar(renderer, scrollbar_rect);
	return inner_rect;
}

void node::BlockPallete::DrawScrollBar(SDL_Renderer* renderer, const SDL_FRect& area)
{
	SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
	SDL_RenderFillRect(renderer, &area);
	float total_height = 
		static_cast<float>(m_palleteProvider->GetElements().size() *
			PalleteData::ElementTotalHeight) + PalleteData::TopPadding;
	float view_area_height = area.h;
	if (total_height < view_area_height)
	{
		return;
	}
	float scrollbar_height = (view_area_height * area.h) / total_height + 1;
	float scrollbar_start_pos = (m_scrollPos * view_area_height) / total_height;
	SDL_FRect draw_area{ area.x, area.y + scrollbar_start_pos, area.w, scrollbar_height };
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &draw_area);
}

void node::BlockPallete::DrawElements(SDL_Renderer* renderer, const SDL_FRect& area)
{
	if (!m_palleteProvider)
	{
		return;
	}

	auto&& elements = m_palleteProvider->GetElements();
	int possible_elements_on_screen = static_cast<int>(area.h / PalleteData::ElementTotalHeight + 2);
	int start_element = static_cast<int>(m_scrollPos / PalleteData::ElementTotalHeight);
	int max_element = std::min(possible_elements_on_screen + start_element, 
		static_cast<int>(elements.size()));
	for (int i = start_element; i < max_element; i++)
	{
		int padding_left = static_cast<int>((area.w - PalleteData::ElementTotalWidth) / 2);
		SDL_FRect element_rect = { area.x + padding_left, 
			area.y + i * PalleteData::ElementTotalHeight - m_scrollPos + 
			PalleteData::TopPadding,
			PalleteData::ElementTotalWidth, PalleteData::ElementTotalHeight };
		if (element_rect.y > area.y + area.h)
		{
			continue;
		}
		DrawElement(renderer, *elements[i], element_rect);
	}
}

void node::BlockPallete::DrawElement(SDL_Renderer* renderer, const PalleteElement& element, const SDL_FRect& area)
{
	SDL_FRect contained_rect = { area.x + PalleteData::ElementWPadding/2, 
		area.y,
		ElementWidth, ElementHeight };
	node::SpaceScreenTransformer transformer{ contained_rect, element.block.GetBounds() };
	element.styler->DrawBlock(renderer, element.block, transformer, false);
	SDL_FRect TextArea = { contained_rect.x, contained_rect.y + contained_rect.h,
		area.w, area.h - contained_rect.h };
	DrawElementText(renderer, element, TextArea);
}

void node::BlockPallete::DrawElementText(SDL_Renderer* renderer,
	const PalleteElement& element, const SDL_FRect& area)
{
	if (!element.text_painter->GetFont())
	{
		element.text_painter->SetFont(GetApp()->getFont().get());
		element.text_painter->SetText(element.block_template);
	}

	SDL_Color Black = { 50, 50, 50, 255 };

	SDL_FRect text_rect = element.text_painter->GetRect(renderer, Black);
	text_rect.x = area.x + area.w / 2 - text_rect.w / 2;
	text_rect.y = area.y + area.h / 2 - text_rect.h / 2 - 10;
	element.text_painter->Draw(renderer, { text_rect.x, text_rect.y }, Black);
}
