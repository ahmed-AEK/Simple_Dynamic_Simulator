#pragma once

#include "toolgui/Widget.hpp"
#include "PalleteProvider.hpp"
#include "SDL_Framework/Utility.hpp"
#include "BlockPallete/BlockPallete.hpp"

namespace node
{

class PalleteProvider;

class PalleteCategoryPicker: public Widget, public SingleObserver<BlockPalleteChange>,
	public SinglePublisher<detail::BlockCategoryClicked>
{
public:
	PalleteCategoryPicker(const SDL_FRect& rect,
		std::shared_ptr<PalleteProvider> provider, TTF_Font* font, Widget* parent);

	void Draw(SDL_Renderer* renderer) override;

	std::shared_ptr<PalleteProvider> GetProvider() const { return m_palleteProvider; }
	void SetProvider(std::shared_ptr<PalleteProvider> provider);
	virtual void OnNotify(BlockPalleteChange& e);
protected:
	MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;
private:
	SDL_FRect DrawPanelBorder(SDL_Renderer* renderer);
	void ResetCategories();

	struct BlockCategorySlot
	{
		TextPainter painter;
		float height = 0;
	};

	std::shared_ptr<PalleteProvider> m_palleteProvider;
	TextPainter m_title_painter;
	std::vector<BlockCategorySlot> m_categories;
	SDL_FRect m_categories_draw_area{};
};

}