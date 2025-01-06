#pragma once

#include "toolgui/Widget.hpp"
#include "BlockPallete/PalleteProvider.hpp"
#include "SDL_Framework/SVGRasterizer.hpp"
#include "BlockPallete/BlockPallete.hpp"

namespace node
{
class BlockClassesManager;
class BlockStylerFactory;

class PalleteBlocksViewer : public Widget, public SinglePublisher<detail::BlockViewerBackClicked>
{
public:
	PalleteBlocksViewer(const SDL_FRect& rect,
		std::shared_ptr<PalleteProvider> provider,
		TTF_Font* font, Widget* parent);

	void Draw(SDL_Renderer* renderer) override;
	std::shared_ptr<PalleteProvider> GetProvider() const { return m_palleteProvider; }
	void SetProvider(std::shared_ptr<PalleteProvider> provider)
	{
		m_palleteProvider = std::move(provider);
	}

	void SetCategory(std::string category);

	static constexpr int ElementHeight = 100;
	static constexpr int ElementWidth = 100;
protected:
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
	void DrawElement(SDL_Renderer* renderer, const PalleteElement& element, const SDL_FRect& area);
	void DrawElementText(SDL_Renderer* renderer, const PalleteElement& element, const SDL_FRect& area);
	std::string m_current_category{ "All" };

	std::shared_ptr<PalleteProvider> m_palleteProvider;
	SVGRasterizer m_back_btn_painter;
	TextPainter m_title_painter;
	float m_scrollPos = 0;
};

}