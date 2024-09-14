#include "GainBlockStyler.hpp"
#include <algorithm>
#include <array>
#include <charconv>

static std::string GetBlockGain(const node::model::BlockModel& model)
{
	using namespace node; 
	const auto& properties = model.GetProperties();
	auto it = std::find_if(properties.begin(), properties.end(), [](const model::BlockProperty& prop) { return prop.name == "Multiplier"; });
	if (it != properties.end())
	{
		if (it->type != model::BlockPropertyType::FloatNumber)
		{
			return {};
		}
		return it->to_string();
	}
	return {};
}

node::GainBlockStyler::GainBlockStyler(const model::BlockModel& model, TTF_Font* font)
	:TextBlockStyler{ GetBlockGain(model), font}
{

}

void node::GainBlockStyler::DrawBlockOutline(SDL_Renderer* renderer, const model::Rect& bounds, const SpaceScreenTransformer& transformer, bool selected)
{
	auto screen_bounds = transformer.SpaceToScreenRect(bounds);
	
	SDL_Color outer_color = selected ? SDL_Color{ 255,165,0,255 } : SDL_Color{ 0,0,0,255 };
	SDL_Color inner_color{ 220,220,220,255 };
	SDL_SetRenderDrawColor(renderer, outer_color.r, outer_color.g, outer_color.b, 255);
	
	std::array<SDL_Vertex, 3> arrow_verts;
	arrow_verts[0].position = SDL_FPoint{ static_cast<float>(screen_bounds.x), static_cast<float>(screen_bounds.y) };
	arrow_verts[1].position = SDL_FPoint{ static_cast<float>(screen_bounds.x), static_cast<float>(screen_bounds.y + screen_bounds.h) };
	arrow_verts[2].position = SDL_FPoint{ static_cast<float>(screen_bounds.x + screen_bounds.w), static_cast<float>(screen_bounds.y + screen_bounds.h/2) };
	for (auto&& vert : arrow_verts)
	{
		vert.color = outer_color;
	}

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
	SDL_RenderGeometry(renderer, nullptr, arrow_verts.data(), 3, nullptr, 0);

	arrow_verts[0].position = SDL_FPoint{ static_cast<float>(screen_bounds.x + 2), static_cast<float>(screen_bounds.y + 4) };
	arrow_verts[1].position = SDL_FPoint{ static_cast<float>(screen_bounds.x + 2), static_cast<float>(screen_bounds.y + screen_bounds.h - 4) };
	arrow_verts[2].position = SDL_FPoint{ static_cast<float>(screen_bounds.x + screen_bounds.w - 4), static_cast<float>(screen_bounds.y + screen_bounds.h / 2) };
	for (auto&& vert : arrow_verts)
	{
		vert.color = inner_color;
	}
	SDL_RenderGeometry(renderer, nullptr, arrow_verts.data(), 3, nullptr, 0);

}

void node::GainBlockStyler::UpdateProperties(const model::BlockModel& model)
{
	SetText(GetBlockGain(model));
}
