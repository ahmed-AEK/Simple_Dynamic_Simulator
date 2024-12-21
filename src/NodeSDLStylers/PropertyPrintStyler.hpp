#pragma once

#include "NodeSDLStylers/TextBlockStyler.hpp"
#include "SDL_Framework/Utility.hpp"

namespace node
{
namespace model
{
	class NodeSceneModel;
}

class PropertyPrintStyler : public TextBlockStyler
{
public:
	static inline const std::string printed_key_text = "Property";
	PropertyPrintStyler(const model::BlockDataCRef& model, TTF_Font* font);
	void UpdateProperties(const model::BlockDataCRef& model) override;
private:
};
}