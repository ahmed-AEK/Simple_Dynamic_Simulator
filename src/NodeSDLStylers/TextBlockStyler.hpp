#pragma once

#include "NodeSDLStylers/DefaultBlockStyler.hpp"
#include "SDL_Framework/Utility.hpp"

namespace node
{

class TextBlockStyler : public DefaultBlockStyler
{
public:
	static inline const std::string key_text = "Text";

	static std::unique_ptr<TextBlockStyler> Create(const model::BlockStyleProperties& properties, TTF_Font* font);
	TextBlockStyler(std::string name, TTF_Font* font);
	void DrawBlockDetails(SDL_Renderer* renderer, const model::Rect& bounds,
		const SpaceScreenTransformer& transformer, bool selected) override;
	void SetText(std::string text);
private:
	TextPainter m_textPainter;

};
}