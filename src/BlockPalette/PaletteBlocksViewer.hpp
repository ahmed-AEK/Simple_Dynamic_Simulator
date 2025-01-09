#pragma once

#include "toolgui/Widget.hpp"
#include "BlockPalette/PaletteProvider.hpp"
#include "SDL_Framework/SVGRasterizer.hpp"
#include "BlockPalette/BlockPalette.hpp"
#include "toolgui/ButtonWidget.hpp"

namespace node
{
class BlockClassesManager;
class BlockStylerFactory;

class PaletteBlocksViewer : public Widget, public SinglePublisher<detail::BlockViewerBackClicked>
{
public:
	PaletteBlocksViewer(const WidgetSize& size,
		std::shared_ptr<PaletteProvider> provider,
		TTF_Font* font, Widget* parent);

	std::shared_ptr<PaletteProvider> GetProvider() const { return m_paletteProvider; }
	void SetProvider(std::shared_ptr<PaletteProvider> provider)
	{
		m_paletteProvider = std::move(provider);
	}

	void SetCategory(std::string category);

	static constexpr int ElementHeight = 100;
	static constexpr int ElementWidth = 100;
protected:
	void OnDraw(SDL::Renderer& renderer) override;

	bool OnScroll(const double amount, const SDL_FPoint& p) override;
	virtual MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;

private:
	SDL_FRect GetOuterRect() const;
	SDL_FRect GetInnerRect() const;

	SDL_FRect GetBackBtnRect() const;
	void DrawHeader(SDL_Renderer* renderer);
	SDL_FRect DrawPanelBorder(SDL_Renderer* renderer);
	void DrawScrollBar(SDL_Renderer* renderer, const SDL_FRect& area);
	void DrawElements(SDL_Renderer* renderer, const SDL_FRect& area);
	void DrawElement(SDL_Renderer* renderer, const PaletteElement& element, const SDL_FRect& area);
	void DrawElementText(SDL_Renderer* renderer, const PaletteElement& element, const SDL_FRect& area);
	std::string m_current_category{ "All" };

	std::shared_ptr<PaletteProvider> m_paletteProvider;
	ButtonWidget m_back_btn;
	TextPainter m_title_painter;
	float m_scrollPos = 0;
};

}