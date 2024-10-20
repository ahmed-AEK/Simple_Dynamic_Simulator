#pragma once

#include "NodeSDLStylers/DefaultBlockStyler.hpp"
#include "SDL_Framework/SVGRasterizer.hpp"

namespace node
{

class SVGBlockStyler: public DefaultBlockStyler
{
public:

	inline static const std::string SVG_PATH_PROPERTY_STRING{ "svg_path" };

	SVGBlockStyler(const node::model::BlockModel& block);
	void DrawBlockDetails(SDL_Renderer* renderer, const model::Rect& bounds,
		const SpaceScreenTransformer& transformer, model::BlockOrientation orientation, bool selected) override;
	virtual void UpdateProperties(const model::BlockModel& model);

private:
	std::optional<SVGRasterizer> m_svg_rasterizer;
	std::optional<SDL_FPoint> m_max_rect;
};
}