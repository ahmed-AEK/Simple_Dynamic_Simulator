#pragma once

#include "toolgui/Widget.hpp"
#include "BlockPallete/PalleteProvider.hpp"
#include <string_view>

namespace node
{

	class BlockPallete : public Widget
	{
	public:
		BlockPallete(const SDL_Rect& rect,
			std::shared_ptr<PalleteProvider> provider, Scene* parent);
		
		void Draw(SDL_Renderer* renderer) override;
		std::shared_ptr<PalleteProvider> GetProvider() const { return m_palleteProvider; }
		void SetProvider(std::shared_ptr<PalleteProvider> provider) 
		{
			m_palleteProvider = provider;
		}

		static constexpr int ElementHeight = 100;
		static constexpr int ElementWidth = 100;
	protected:
		bool OnScroll(const double amount, const SDL_Point& p) override;
		virtual MI::ClickEvent OnLMBDown(const SDL_Point& current_mouse_point);

	private:
		SDL_Rect DrawPanelBorder(SDL_Renderer* renderer);
		void DrawScrollBar(SDL_Renderer* renderer, const SDL_Rect& area);
		void DrawElements(SDL_Renderer* renderer, const SDL_Rect& area);
		void DrawElement(SDL_Renderer* renderer, const PalleteElement& element, const SDL_Rect& area);
		void DrawElementText(SDL_Renderer* renderer, const std::string& name, const SDL_Rect& area);


		std::shared_ptr<PalleteProvider> m_palleteProvider;
		int m_scrollPos = 0;
	};


}