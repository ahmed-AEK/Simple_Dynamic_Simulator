#include "SVGBlockStyler.hpp"


static std::string GetModelSVGPath(const node::model::BlockModel& model)
{
	using namespace node;

	auto&& properties = model.GetStylerProperties();
	auto it = properties.properties.find(SVGBlockStyler::SVG_PATH_PROPERTY_STRING);

	std::string text;
	if (it != properties.properties.end())
	{
		text = it->second;
	}
	else
	{
		SDL_Log("svg_path not found!");
	}
	return text;
}

node::SVGBlockStyler::SVGBlockStyler(const node::model::BlockModel& block)
	:m_svg_rasterizer{ {GetModelSVGPath(block), 0, 0} }
{
}

void node::SVGBlockStyler::DrawBlockDetails(SDL_Renderer* renderer, const model::Rect& bounds, const SpaceScreenTransformer& transformer, model::BlockOrientation orientation, bool selected)
{
	if (!m_svg_rasterizer)
	{
		return;
	}

	model::Rect bound_inner = bounds;
	bound_inner.x += SocketLength;
	bound_inner.y += SocketLength;
	bound_inner.w -= 2 * SocketLength;
	bound_inner.h -= 2 * SocketLength;

	SDL_Rect screen_rect = transformer.SpaceToScreenRect(bound_inner);
	SDL_Point screen_center{ screen_rect.x + screen_rect.w / 2, screen_rect.y + screen_rect.h / 2 };
	if (!m_max_rect)
	{
		auto svg_size = m_svg_rasterizer->GetSVGSize();
		if (svg_size)
		{
			m_max_rect = *svg_size;
		}
	}

	screen_rect.x += 5;
	screen_rect.y += 5;
	screen_rect.w -= 10;
	screen_rect.h -= 10;

	int width = screen_rect.w;
	int height = screen_rect.h;
	if (m_max_rect)
	{
		width = std::min(static_cast<int>(m_max_rect->x), width);
		height = std::min(static_cast<int>(m_max_rect->y), height);
	}

	SDL_Point start_point{ screen_center.x - width / 2, screen_center.y - height / 2 };
	m_svg_rasterizer->SetSize(width, height);
	if (!m_svg_rasterizer->Draw(renderer, start_point.x, start_point.y))
	{
		m_svg_rasterizer = std::nullopt;
	}

	UNUSED_PARAM(orientation);
	UNUSED_PARAM(selected);
}

void node::SVGBlockStyler::UpdateProperties(const model::BlockModel& model)
{
	if (!m_svg_rasterizer)
	{
		m_svg_rasterizer.emplace(GetModelSVGPath(model), 0, 0);
	}
	else
	{
		m_svg_rasterizer->SetSVGPath(GetModelSVGPath(model));
	}
	m_max_rect = std::nullopt;
}
