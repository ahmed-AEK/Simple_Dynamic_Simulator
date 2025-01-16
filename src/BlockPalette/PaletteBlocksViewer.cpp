#include "PaletteBlocksViewer.hpp"
#include <cassert>
#include "SDL_Framework/SDLCPP.hpp"
#include "toolgui/Scene.hpp"
#include "toolgui/Application.hpp"
#include "NodeSDLStylers/BlockStylerFactory.hpp"

namespace PaletteData {
	static constexpr int TopPadding = 20;
	static constexpr int ElementHPadding = 50;
	static constexpr int ElementTotalHeight = node::PaletteBlocksViewer::ElementHeight + ElementHPadding;
	static constexpr int ElementWPadding = 10;
	static constexpr int ElementTotalWidth = node::PaletteBlocksViewer::ElementWidth + ElementWPadding;
	static constexpr int scrollbarWidth = 10;
}

node::PaletteBlocksViewer::PaletteBlocksViewer(const WidgetSize& size,
	std::shared_ptr<PaletteProvider> provider, TTF_Font* font,  Widget* parent)
	: Widget(size, parent), m_paletteProvider{ provider },
	m_back_btn{ WidgetSize{35,35}, "back", "assets/arrow_left.svg" , [this] { this->Notify(detail::BlockViewerBackClicked{});}, this },
	m_title_painter{ font }
{
	m_back_btn.SetPosition({20, 5});
	assert(font);
}

void node::PaletteBlocksViewer::OnDraw(SDL::Renderer& renderer)
{
	SDL_FRect inner_area = DrawPanelBorder(renderer);
	DrawHeader(renderer);
	SDL_Rect inner_area_int = ToRect(inner_area);
	SDL_SetRenderClipRect(renderer, &inner_area_int);
	DrawElements(renderer, inner_area);
	SDL_SetRenderClipRect(renderer, nullptr);
}

void node::PaletteBlocksViewer::SetCategory(std::string category)
{
	m_current_category = std::move(category); 
	m_title_painter.SetText(m_current_category);
	m_scrollPos = 0;
}

bool node::PaletteBlocksViewer::OnScroll(const double amount, const SDL_FPoint& p)
{
	if (!m_paletteProvider)
	{
		return false;
	}

	SDL_FRect inner_rect = GetInnerRect();

	if (SDL_PointInRectFloat(&p, &inner_rect))
	{
		float elements_height = static_cast<float>(
			m_paletteProvider->GetCategoryElements(m_current_category)->size() * PaletteData::ElementTotalHeight + PaletteData::TopPadding);
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

MI::ClickEvent node::PaletteBlocksViewer::OnLMBDown(MouseButtonEvent& e)
{
	SDL_FPoint current_mouse_point{ e.point() };

	SDL_FRect inner_area = GetInnerRect();
	float Hpad_size = inner_area.w - PaletteData::ElementTotalWidth;
	inner_area.x += Hpad_size / 2;
	inner_area.w -= Hpad_size / 2;

	if (!SDL_PointInRectFloat(&current_mouse_point, &inner_area))
	{
		return MI::ClickEvent::NONE;
	}
	float selected_y = current_mouse_point.y - inner_area.y + m_scrollPos -
		PaletteData::TopPadding;
	int selected_item_index = static_cast<int>(selected_y / PaletteData::ElementTotalHeight);

	auto&& palette_elements = *m_paletteProvider->GetCategoryElements(m_current_category);
	assert(selected_item_index >= 0);
	assert(static_cast<size_t>(selected_item_index) < palette_elements.size());

	auto&& selected_element = palette_elements[selected_item_index];
	auto block_data_ref = model::BlockDataCRef{ selected_element->block, {} };
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
				m_paletteProvider->GetStylerFactory().GetStyler(selected_element->block.GetStyler(), block_data_ref)
			},
			model::BlockData{selected_element->data}
		});

	return MI::ClickEvent::NONE;
}

SDL_FRect node::PaletteBlocksViewer::GetOuterRect() const
{
	SDL_FRect ret = GetSize().ToRect();
	ret.x += 5;
	ret.w -= 10;
	ret.y += 5 + 40;
	ret.h -= 10 + 40;
	return ret;
}

SDL_FRect node::PaletteBlocksViewer::GetInnerRect() const
{
	SDL_FRect ret = GetSize().ToRect();
	ret.x += 7;
	ret.w -= 14;
	ret.y += 7 + 40;
	ret.h -= 14 + 40;
	return ret;
}

SDL_FRect node::PaletteBlocksViewer::GetBackBtnRect() const
{
	SDL_FRect btn_rect = GetSize().ToRect();
	btn_rect.x += 20;
	btn_rect.y += 5;
	btn_rect.w = 35;
	btn_rect.h = 35;
	return btn_rect;
}

void node::PaletteBlocksViewer::DrawHeader(SDL_Renderer* renderer)
{
	SDL_FRect rect = GetSize().ToRect();

	SDL_Color Black{ 50,50,50,255 };
	SDL_FRect title_rect = m_title_painter.GetRect(renderer, Black);
	title_rect.x = rect.x + rect.w / 2 - title_rect.w / 2;
	title_rect.y = rect.y + 10;
	m_title_painter.Draw(renderer, { title_rect.x, title_rect.y }, Black);
}

SDL_FRect node::PaletteBlocksViewer::DrawPanelBorder(SDL_Renderer* renderer)
{
	auto draw_area = GetInnerRect();
	auto outer_rect = draw_area;
	outer_rect.x -= 2;
	outer_rect.y -= 2;
	outer_rect.w += 4;
	outer_rect.h += 4;
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &outer_rect);

	SDL_FRect inner_rect = draw_area;
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(renderer, &draw_area);
	inner_rect.w -= PaletteData::scrollbarWidth;
	SDL_FRect scrollbar_rect = { inner_rect.x + inner_rect.w, inner_rect.y,
	PaletteData::scrollbarWidth, inner_rect.h };
	DrawScrollBar(renderer, scrollbar_rect);
	return inner_rect;
}

void node::PaletteBlocksViewer::DrawScrollBar(SDL_Renderer* renderer, const SDL_FRect& area)
{
	SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
	SDL_RenderFillRect(renderer, &area);
	float total_height =
		static_cast<float>(m_paletteProvider->GetCategoryElements(m_current_category)->size() *
			PaletteData::ElementTotalHeight) + PaletteData::TopPadding;
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

void node::PaletteBlocksViewer::DrawElements(SDL_Renderer* renderer, const SDL_FRect& area)
{
	if (!m_paletteProvider)
	{
		return;
	}

	auto&& elements = *m_paletteProvider->GetCategoryElements(m_current_category);
	int possible_elements_on_screen = static_cast<int>(area.h / PaletteData::ElementTotalHeight + 2);
	int start_element = static_cast<int>(m_scrollPos / PaletteData::ElementTotalHeight);
	int max_element = std::min(possible_elements_on_screen + start_element,
		static_cast<int>(elements.size()));
	for (int i = start_element; i < max_element; i++)
	{
		int padding_left = static_cast<int>((area.w - PaletteData::ElementTotalWidth) / 2);
		SDL_FRect element_rect = { area.x + padding_left,
			area.y + i * PaletteData::ElementTotalHeight - m_scrollPos +
			PaletteData::TopPadding,
			PaletteData::ElementTotalWidth, PaletteData::ElementTotalHeight };
		if (element_rect.y > area.y + area.h)
		{
			continue;
		}
		DrawElement(renderer, *elements[i], element_rect);
	}
}

void node::PaletteBlocksViewer::DrawElement(SDL_Renderer* renderer, const PaletteElement& element, const SDL_FRect& area)
{
	SDL_FRect contained_rect = { area.x + PaletteData::ElementWPadding / 2,
		area.y,
		ElementWidth, ElementHeight };
	node::SpaceScreenTransformer transformer{ contained_rect, element.block.GetBounds() };
	element.styler->DrawBlock(renderer, element.block, transformer, false);
	SDL_FRect TextArea = { contained_rect.x, contained_rect.y + contained_rect.h,
		area.w, area.h - contained_rect.h };
	DrawElementText(renderer, element, TextArea);
}

void node::PaletteBlocksViewer::DrawElementText(SDL_Renderer* renderer,
	const PaletteElement& element, const SDL_FRect& area)
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
