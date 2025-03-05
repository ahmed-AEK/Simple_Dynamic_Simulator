#pragma once

#include "toolgui/Widget.hpp"
#include "BlockPalette/PaletteProvider.hpp"
#include "SDL_Framework/SVGRasterizer.hpp"
#include "BlockPalette/BlockPalette.hpp"
#include "toolgui/ButtonWidget.hpp"
#include "toolgui/ScrollView.hpp"

namespace node
{
class BlockClassesManager;
class BlockStylerFactory;

namespace palette_viewer
{

class BlocksElementsViewer: public Widget
{
public:
	BlocksElementsViewer(const WidgetSize& size,
		std::shared_ptr<PaletteProvider> provider,
		TTF_Font* font, Widget* parent);
	void SetCategory(std::string category);
	const std::string& GetCategory() const { return m_current_category; }
	void SetProvider(std::shared_ptr<PaletteProvider> provider);
	
	void UpdateElements();
		
	static constexpr int ElementHeight = 100;
	static constexpr int ElementWidth = 100;
protected:
	MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;
	void OnDraw(SDL::Renderer& renderer) override;

private:
	void ResizeToElementsHeight();
	void DrawElement(SDL_Renderer* renderer, const PaletteElement& element, const SDL_FRect& area);
	void DrawElementText(SDL_Renderer* renderer, const PaletteElement& element, const SDL_FRect& area);

	std::string m_current_category{ "All" };
	std::shared_ptr<PaletteProvider> m_paletteProvider;
	TTF_Font* m_font;
};

}

class PaletteBlocksViewer : public Widget, public SinglePublisher<detail::BlockViewerBackClicked>, public SingleObserver<BlockPaletteChange>
{
public:
	PaletteBlocksViewer(const WidgetSize& size,
		std::shared_ptr<PaletteProvider> provider,
		TTF_Font* font, Widget* parent);

	std::shared_ptr<PaletteProvider> GetProvider() const { return m_paletteProvider; }
	void SetProvider(std::shared_ptr<PaletteProvider> provider);

	void SetCategory(std::string category);
	void OnNotify(BlockPaletteChange& e) override;

	static constexpr int ElementHeight = palette_viewer::BlocksElementsViewer::ElementHeight;
	static constexpr int ElementWidth = palette_viewer::BlocksElementsViewer::ElementWidth;
protected:
	void OnDraw(SDL::Renderer& renderer) override;

	void OnSetSize(const WidgetSize& size) override;
private:
	SDL_FRect GetOuterRect() const;
	SDL_FRect GetInnerRect() const;

	SDL_FRect GetBackBtnRect() const;
	void DrawHeader(SDL_Renderer* renderer);
	void DrawInnerBorders(SDL_Renderer* renderer);
	RoundRectPainter m_borders_outer_painter;
	RoundRectPainter m_borders_inner_painter;

	ScrollView m_scrollview;
	std::string m_current_category{ "All" };
	palette_viewer::BlocksElementsViewer* m_elements_viewer;
	std::shared_ptr<PaletteProvider> m_paletteProvider;

	ButtonWidget m_back_btn;
	TextPainter m_title_painter;
};

}