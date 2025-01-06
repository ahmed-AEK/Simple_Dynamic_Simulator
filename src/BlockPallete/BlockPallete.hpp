#pragma once

#include "toolgui/Widget.hpp"
#include "toolgui/StackedWidget.hpp"
#include "NodeModels/Observer.hpp"

namespace node
{

class PalleteProvider;
class PalleteBlocksViewer;
class PalleteCategoryPicker;

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

class BlockPallete : public Widget, public SingleObserver<detail::BlockCategoryClicked>,
	public SingleObserver<detail::BlockViewerBackClicked>
{
public:
	BlockPallete(const SDL_FRect& rect,
		std::shared_ptr<PalleteProvider> provider, TTF_Font* font, Widget* parent);
		
	void Draw(SDL_Renderer* renderer) override;
	std::shared_ptr<PalleteProvider> GetProvider() const { return m_palleteProvider; }
	void SetProvider(std::shared_ptr<PalleteProvider> provider);
	void OnNotify(detail::BlockCategoryClicked& e) override;
	void OnNotify(detail::BlockViewerBackClicked& e) override;

protected:
	void OnSetRect(const SDL_FRect& rect) override;
	Widget* OnGetInteractableAtPoint(const SDL_FPoint& point) override;

private:
	StackedWidget m_stacked_widget;
	std::shared_ptr<PalleteProvider> m_palleteProvider;
	PalleteBlocksViewer* m_palleteBlocksViewer = nullptr;
	PalleteCategoryPicker* m_palleteCategoryPicker = nullptr;
};

}