#pragma once

#include "toolgui/Widget.hpp"
#include "toolgui/StackedWidget.hpp"
#include "NodeModels/Observer.hpp"

namespace node
{

class PaletteProvider;
class PaletteBlocksViewer;
class PaletteCategoryPicker;

namespace detail
{
	struct BlockCategoryClicked
	{
		std::string category_name;
	};

	struct BlockViewerBackClicked
	{

	};
}

class BlockPalette : public Widget, public SingleObserver<detail::BlockCategoryClicked>,
	public SingleObserver<detail::BlockViewerBackClicked>
{
public:
	BlockPalette(const WidgetSize& size,
		std::shared_ptr<PaletteProvider> provider, TTF_Font* font, Widget* parent);
		
	std::shared_ptr<PaletteProvider> GetProvider() const { return m_paletteProvider; }
	void SetProvider(std::shared_ptr<PaletteProvider> provider);
	void OnNotify(detail::BlockCategoryClicked& e) override;
	void OnNotify(detail::BlockViewerBackClicked& e) override;

protected:
	void OnSetSize(const WidgetSize& size) override;

private:
	StackedWidget m_stacked_widget;
	std::shared_ptr<PaletteProvider> m_paletteProvider;
	PaletteBlocksViewer* m_paletteBlocksViewer = nullptr;
	PaletteCategoryPicker* m_paletteCategoryPicker = nullptr;
};

}