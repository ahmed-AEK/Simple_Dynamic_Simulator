#pragma once

#include "NodeSDLStylers/TextBlockStyler.hpp"

namespace node
{
namespace model
{
	class NodeSceneModel;
}

class GainBlockStyler: public TextBlockStyler
{
public:
	GainBlockStyler(const model::BlockDataCRef& model, TTF_Font* font);
	void DrawBlockOutline(SDL_Renderer* renderer, const model::Rect& bounds,
		const SpaceScreenTransformer& transformer, model::BlockOrientation orientation, bool selected) override;
	void UpdateProperties(const model::BlockDataCRef& model) override;
private:
};

}