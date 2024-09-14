#pragma once

#include "NodeSDLStylers/TextBlockStyler.hpp"
#include "SDL_Framework/Utility.hpp"

namespace node
{

class PropertyPrintStyler : public TextBlockStyler
{
public:
	static inline const std::string printed_key_text = "Property";
	static std::unique_ptr<PropertyPrintStyler> Create(const model::BlockModel& model, TTF_Font* font);
	PropertyPrintStyler(std::string name, TTF_Font* font);
	void UpdateProperties(const model::BlockModel& model) override;
};
}