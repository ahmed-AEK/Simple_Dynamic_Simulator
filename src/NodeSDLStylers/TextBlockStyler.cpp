#include "TextBlockStyler.hpp"

std::unique_ptr<node::TextBlockStyler> node::TextBlockStyler::Create(const model::BlockStyleProperties& properties, TTF_Font* font)
{
	auto it = properties.properties.find(key_text);
	assert(it != properties.properties.end());
	std::string text;
	assert(it != properties.properties.end());
	if (it != properties.properties.end())
	{
		text = it->second;
	}
	return std::make_unique<node::TextBlockStyler>(text, font);
}

node::TextBlockStyler::TextBlockStyler(std::string name, TTF_Font* font)
	:m_textPainter{font}
{
	m_textPainter.SetText(std::move(name));
}

void node::TextBlockStyler::DrawBlockDetails(SDL_Renderer* renderer, const model::Rect& bounds, const SpaceScreenTransformer& transformer, 
	model::BlockOrientation orientation, bool selected)
{
	SDL_Color Black{ 50,50,50,255 };
	SDL_FRect screen_rect = transformer.SpaceToScreenRect(bounds);
	auto rect = m_textPainter.GetRect(renderer, Black);
	SDL_FPoint text_start{ screen_rect.x + screen_rect.w / 2 - rect.w / 2, screen_rect.y + screen_rect.h / 2 - rect.h / 2};
	m_textPainter.Draw(renderer, text_start, Black);
	UNUSED_PARAM(selected);
	UNUSED_PARAM(orientation);
}

void node::TextBlockStyler::SetText(std::string text)
{
	m_textPainter.SetText(std::move(text));
}
