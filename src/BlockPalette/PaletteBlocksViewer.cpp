#include "PaletteBlocksViewer.hpp"
#include <cassert>
#include "SDL_Framework/SDLCPP.hpp"
#include "toolgui/Scene.hpp"
#include "toolgui/Application.hpp"
#include "NodeSDLStylers/BlockStylerFactory.hpp"

namespace PaletteData {
	static constexpr int TopPadding = 20;
	static constexpr int ElementHPadding = 50;
	static constexpr int ElementTotalHeight = node::palette_viewer::BlocksElementsViewer::ElementHeight + ElementHPadding;
	static constexpr int ElementWPadding = 10;
	static constexpr int ElementTotalWidth = node::palette_viewer::BlocksElementsViewer::ElementWidth + ElementWPadding;
	static constexpr int scrollbarWidth = 10;
}

node::PaletteBlocksViewer::PaletteBlocksViewer(const WidgetSize& size,
	std::shared_ptr<PaletteProvider> provider, TTF_Font* font,  Widget* parent)
	: Widget(size, parent), m_scrollview{ {10,10}, this}, m_paletteProvider{provider},
	m_back_btn{ WidgetSize{35,35}, "back", "assets/arrow_left.svg" , [this] { this->Notify(detail::BlockViewerBackClicked{});}, this },
	m_title_painter{ font }
{
	auto child_rect = GetInnerRect();
	m_scrollview.SetPosition({ child_rect.x, child_rect.y });
	m_scrollview.SetSize({ child_rect.w, child_rect.h });

	auto elements_viewer = std::make_unique<palette_viewer::BlocksElementsViewer>(
		WidgetSize{ child_rect.w, 1000 }, provider, font, nullptr);
	m_elements_viewer = elements_viewer.get();
	m_scrollview.SetWidget(std::move(elements_viewer));
		
	m_back_btn.SetPosition({20, 5});
	assert(font);
}

void node::PaletteBlocksViewer::OnDraw(SDL::Renderer& renderer)
{
	DrawHeader(renderer);
	DrawInnerBorders(renderer);
}

void node::PaletteBlocksViewer::SetProvider(std::shared_ptr<PaletteProvider> provider)
{
	m_paletteProvider = provider;
	if (m_elements_viewer)
	{
		m_elements_viewer->SetProvider(std::move(provider));
	}
}

void node::PaletteBlocksViewer::SetCategory(std::string category)
{
	m_elements_viewer->SetCategory(category);
	m_current_category = std::move(category); 
	m_title_painter.SetText(m_current_category);
	m_scrollview.RequestPosition(0);
}

void node::PaletteBlocksViewer::OnSetSize(const WidgetSize& size)
{
	Widget::OnSetSize(size);
	auto child_rect = GetInnerRect();
	m_scrollview.SetPosition({ child_rect.x, child_rect.y });
	m_scrollview.SetSize({ child_rect.w, child_rect.h });
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

void node::PaletteBlocksViewer::DrawInnerBorders(SDL_Renderer* renderer)
{
	const auto& inner_rect = GetInnerRect();
	auto outer_rect = inner_rect;
	outer_rect.x -= 2;
	outer_rect.y -= 2;
	outer_rect.w += 4;
	outer_rect.h += 4;
	ThickFilledRoundRect(renderer, outer_rect, 8, 2, 
		SDL_Color{ 0,0,0,255 }, SDL_Color{ 255,255,255,255 }, m_borders_outer_painter, m_borders_inner_painter);

}

node::palette_viewer::BlocksElementsViewer::BlocksElementsViewer(const WidgetSize& size, std::shared_ptr<PaletteProvider> provider, TTF_Font* font, Widget* parent)
	:Widget{size, parent}, m_paletteProvider{std::move(provider)}, m_font{font}
{
}

void node::palette_viewer::BlocksElementsViewer::SetCategory(std::string category)
{
	m_current_category = std::move(category);
	ResizeToElementsHeight();
}

void node::palette_viewer::BlocksElementsViewer::SetProvider(std::shared_ptr<PaletteProvider> provider)
{
	m_paletteProvider = std::move(provider);
	ResizeToElementsHeight();
}

MI::ClickEvent node::palette_viewer::BlocksElementsViewer::OnLMBDown(MouseButtonEvent& e)
{
	SDL_FPoint current_mouse_point{ e.point() };

	SDL_FRect inner_area = GetSize().ToRect();
	float Hpad_size = inner_area.w - PaletteData::ElementTotalWidth;
	inner_area.x += Hpad_size / 2;
	inner_area.w -= Hpad_size / 2;

	if (!SDL_PointInRectFloat(&current_mouse_point, &inner_area))
	{
		return MI::ClickEvent::NONE;
	}
	float selected_y = current_mouse_point.y - PaletteData::TopPadding;
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

	return MI::ClickEvent::CLICKED;
}

void node::palette_viewer::BlocksElementsViewer::OnDraw(SDL::Renderer& renderer)
{
	if (!m_paletteProvider)
	{
		return;
	}

	auto rect = GetSize().ToRect();

	auto elements_ptr = m_paletteProvider->GetCategoryElements(m_current_category);
	assert(elements_ptr);
	auto&& elements = *elements_ptr;
	int possible_elements_on_screen = static_cast<int>(rect.h / PaletteData::ElementTotalHeight + 2);
	int start_element = 0;
	int max_element = std::min(possible_elements_on_screen + start_element,
		static_cast<int>(elements.size()));
	auto renderRect = ToFRect(renderer.GetClipRect());
	for (int i = start_element; i < max_element; i++)
	{
		int padding_left = static_cast<int>((rect.w - PaletteData::ElementTotalWidth) / 2);
		SDL_FRect element_rect = { rect.x + padding_left,
			rect.y + i * PaletteData::ElementTotalHeight +
			PaletteData::TopPadding,
			PaletteData::ElementTotalWidth, PaletteData::ElementTotalHeight };
		if (!SDL_HasRectIntersectionFloat(&renderRect, &element_rect))
		{
			continue;
		}
		DrawElement(renderer, *elements[i], element_rect);
	}
}

void node::palette_viewer::BlocksElementsViewer::ResizeToElementsHeight()
{
	if (!m_paletteProvider)
	{
		return;
	}
	auto elements_ptr = m_paletteProvider->GetCategoryElements(m_current_category);
	assert(elements_ptr);
	auto&& elements = *elements_ptr;
	auto next_height = PaletteData::ElementTotalHeight * elements.size() + PaletteData::TopPadding;
	SetSize({ GetSize().w, static_cast<float>(next_height)});
}

void node::palette_viewer::BlocksElementsViewer::DrawElement(SDL_Renderer* renderer, const PaletteElement& element, const SDL_FRect& area)
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

void node::palette_viewer::BlocksElementsViewer::DrawElementText(SDL_Renderer* renderer, const PaletteElement& element, const SDL_FRect& area)
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
