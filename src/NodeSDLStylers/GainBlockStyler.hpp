#pragma once

#include "NodeSDLStylers/TextBlockStyler.hpp"

namespace node
{

class GainBlockStyler: public TextBlockStyler
{
public:
	GainBlockStyler(const model::BlockModel& model, TTF_Font* font);
	void DrawBlockOutline(SDL_Renderer* renderer, const model::Rect& bounds,
		const SpaceScreenTransformer& transformer, model::BlockOrientation orientation, bool selected) override;
	void UpdateProperties(const model::BlockModel& model) override;
private:
	
};

}