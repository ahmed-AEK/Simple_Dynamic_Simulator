#pragma once

#include "toolgui/Widget.hpp"
#include "PaletteProvider.hpp"
#include "SDL_Framework/Utility.hpp"
#include "BlockPalette/BlockPalette.hpp"

namespace node
{

class PaletteProvider;

class PaletteCategoryPicker: public Widget, public SingleObserver<BlockPaletteChange>,
	public SinglePublisher<detail::BlockCategoryClicked>
{
public:
	PaletteCategoryPicker(const WidgetSize& size,
		std::shared_ptr<PaletteProvider> provider, TTF_Font* font, Widget* parent);


	std::shared_ptr<PaletteProvider> GetProvider() const { return m_paletteProvider; }
	void SetProvider(std::shared_ptr<PaletteProvider> provider);
	void OnNotify(BlockPaletteChange& e) override;
protected:
	void OnDraw(SDL::Renderer& renderer) override;

	MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;
private:
	SDL_FRect DrawPanelBorder(SDL_Renderer* renderer);
	void ResetCategories();

	struct BlockCategorySlot
	{
		TextPainter painter;
		float height = 0;
	};

	std::shared_ptr<PaletteProvider> m_paletteProvider;
	TTF_Font* m_items_font = nullptr;
	std::vector<BlockCategorySlot> m_categories;
	SDL_FRect m_categories_draw_area{};
};

}