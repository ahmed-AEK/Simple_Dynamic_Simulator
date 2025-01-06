#include "BlockPallete.hpp"
#include "PalleteBlocksViewer.hpp"
#include "PalleteCategoryPicker.hpp"

node::BlockPallete::BlockPallete(const SDL_FRect& rect, 
	std::shared_ptr<PalleteProvider> provider, TTF_Font* font, Widget* parent)
	:Widget{rect, parent}, m_stacked_widget{rect, this}
{
	SetFocusProxy(&m_stacked_widget);
	{
		auto category_picker = std::make_unique<PalleteCategoryPicker>(rect, provider, font, this);
		m_palleteCategoryPicker = category_picker.get();
		m_palleteCategoryPicker->Attach(*this);
		m_stacked_widget.AddWidget(std::move(category_picker));
	}
	{
		auto pallete_viewer = std::make_unique<PalleteBlocksViewer>(rect, provider, font, this);
		m_palleteBlocksViewer = pallete_viewer.get();
		m_palleteBlocksViewer->Attach(*this);
		m_stacked_widget.AddWidget(std::move(pallete_viewer));
	}
	m_stacked_widget.SetCurrentIndex(0);
}

void node::BlockPallete::Draw(SDL_Renderer* renderer)
{
	m_stacked_widget.Draw(renderer);
}

void node::BlockPallete::SetProvider(std::shared_ptr<PalleteProvider> provider)
{
	m_palleteBlocksViewer->SetProvider(provider);
	m_palleteProvider = std::move(provider);
}

void node::BlockPallete::OnNotify(detail::BlockCategoryClicked& e)
{
	SDL_Log("Category clicked: %s", e.category_name.c_str());
	m_palleteBlocksViewer->SetCategory(e.category_name);
	m_stacked_widget.SetCurrentIndex(1);
}

void node::BlockPallete::OnNotify(detail::BlockViewerBackClicked& e)
{
	UNUSED_PARAM(e);
	SDL_Log("Back clicked");
	m_stacked_widget.SetCurrentIndex(0);
}

void node::BlockPallete::OnSetRect(const SDL_FRect& rect)
{
	Widget::OnSetRect(rect);
	m_stacked_widget.SetRect(rect);
}

node::Widget* node::BlockPallete::OnGetInteractableAtPoint(const SDL_FPoint& point)
{
	return m_stacked_widget.GetInteractableAtPoint(point);
}
