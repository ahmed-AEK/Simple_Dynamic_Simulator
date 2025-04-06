#include "BlockPalette.hpp"
#include "PaletteBlocksViewer.hpp"
#include "PaletteCategoryPicker.hpp"

node::BlockPalette::BlockPalette(const WidgetSize& size, 
	std::shared_ptr<PaletteProvider> provider, TTF_Font* font, Widget* parent)
	:Widget{size, parent}, m_stacked_widget{size, this}
{
	SetFocusProxy(&m_stacked_widget);
	{
		auto category_picker = std::make_unique<PaletteCategoryPicker>(size, provider, font, this);
		m_paletteCategoryPicker = category_picker.get();
		m_paletteCategoryPicker->Attach(*this);
		m_stacked_widget.AddWidget(std::move(category_picker));
	}
	{
		auto palette_viewer = std::make_unique<PaletteBlocksViewer>(size, provider, font, this);
		m_paletteBlocksViewer = palette_viewer.get();
		m_paletteBlocksViewer->Attach(*this);
		m_stacked_widget.AddWidget(std::move(palette_viewer));
	}
	m_stacked_widget.SetCurrentIndex(0);
}

void node::BlockPalette::SetProvider(std::shared_ptr<PaletteProvider> provider)
{
	m_paletteBlocksViewer->SetProvider(provider);
	m_paletteProvider = std::move(provider);
}

void node::BlockPalette::OnNotify(detail::BlockCategoryClicked& e)
{
	m_logger.LogDebug("Category clicked: {}", e.category_name);
	m_paletteBlocksViewer->SetCategory(e.category_name);
	m_stacked_widget.SetCurrentIndex(1);
}

void node::BlockPalette::OnNotify(detail::BlockViewerBackClicked& e)
{
	UNUSED_PARAM(e);
	m_logger.LogDebug("Back clicked");
	m_stacked_widget.SetCurrentIndex(0);
}

void node::BlockPalette::OnSetSize(const WidgetSize& size)
{
	Widget::OnSetSize(size);
	m_stacked_widget.SetSize(size);
}
