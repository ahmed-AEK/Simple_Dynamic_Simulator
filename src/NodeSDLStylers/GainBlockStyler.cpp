#include "GainBlockStyler.hpp"
#include "NodeModels/NodeScene.hpp"
#include "NodeModels/BlockData.hpp"

#include <algorithm>
#include <array>
#include <charconv>

static std::string GetBlockGain(const node::model::FunctionalBlockData* data)
{
	using namespace node; 
	if (!data)
	{
		return "NO DATA";
	}

	const auto& properties = data->properties;
	auto it = std::find_if(properties.begin(), properties.end(), [](const model::BlockProperty& prop) { return prop.name == "Multiplier"; });
	if (it != properties.end())
	{
		if (it->GetType() != model::BlockPropertyType::FloatNumber)
		{
			return {};
		}
		return it->to_string();
	}
	return {};
}

node::GainBlockStyler::GainBlockStyler(const model::BlockDataCRef& model, TTF_Font* font)
	:TextBlockStyler{ GetBlockGain(model.GetFunctionalData()), font}
{

}

void node::GainBlockStyler::DrawBlockOutline(SDL::Renderer& renderer, const model::Rect& bounds, const SpaceScreenTransformer& transformer,
	model::BlockOrientation orientation, bool selected)
{
	auto screen_bounds = transformer.SpaceToScreenRect(bounds);
	
	const SDL_Color outer_color = selected ?
		renderer.GetColor(ColorRole::block_outline_selected) : renderer.GetColor(ColorRole::block_outline);
	const SDL_Color inner_color = renderer.GetColor(ColorRole::block_background);
	SDL_SetRenderDrawColor(renderer, outer_color.r, outer_color.g, outer_color.b, 255);
	
	SDL_FColor outer_color_f{ToFColor(outer_color)};
	SDL_FColor inner_color_f{ToFColor(inner_color)};

	std::array<SDL_Vertex, 3> arrow_verts;
	switch (orientation)
	{
		using enum model::BlockOrientation;
	case LeftToRight:
	{
		arrow_verts[0].position = SDL_FPoint{ static_cast<float>(screen_bounds.x), static_cast<float>(screen_bounds.y) };
		arrow_verts[1].position = SDL_FPoint{ static_cast<float>(screen_bounds.x), static_cast<float>(screen_bounds.y + screen_bounds.h) };
		arrow_verts[2].position = SDL_FPoint{ static_cast<float>(screen_bounds.x + screen_bounds.w), static_cast<float>(screen_bounds.y + screen_bounds.h / 2) };
		break;
	}
	case TopToBottom:
	{
		arrow_verts[0].position = SDL_FPoint{ static_cast<float>(screen_bounds.x), static_cast<float>(screen_bounds.y) };
		arrow_verts[1].position = SDL_FPoint{ static_cast<float>(screen_bounds.x + screen_bounds.w), static_cast<float>(screen_bounds.y) };
		arrow_verts[2].position = SDL_FPoint{ static_cast<float>(screen_bounds.x + screen_bounds.w / 2), static_cast<float>(screen_bounds.y + screen_bounds.h) };
		break;
	}
	case RightToLeft:
	{
		arrow_verts[0].position = SDL_FPoint{ static_cast<float>(screen_bounds.x + screen_bounds.w), static_cast<float>(screen_bounds.y) };
		arrow_verts[1].position = SDL_FPoint{ static_cast<float>(screen_bounds.x + screen_bounds.w), static_cast<float>(screen_bounds.y + screen_bounds.h) };
		arrow_verts[2].position = SDL_FPoint{ static_cast<float>(screen_bounds.x), static_cast<float>(screen_bounds.y + screen_bounds.h / 2) };
		break;
	}
	case BottomToTop:
	{
		arrow_verts[0].position = SDL_FPoint{ static_cast<float>(screen_bounds.x), static_cast<float>(screen_bounds.y + screen_bounds.h) };
		arrow_verts[1].position = SDL_FPoint{ static_cast<float>(screen_bounds.x + screen_bounds.w), static_cast<float>(screen_bounds.y + screen_bounds.h) };
		arrow_verts[2].position = SDL_FPoint{ static_cast<float>(screen_bounds.x + screen_bounds.w / 2), static_cast<float>(screen_bounds.y) };
		break;
	}
	}
	for (auto&& vert : arrow_verts)
	{
		vert.color = outer_color_f;
	}

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
	SDL_RenderGeometry(renderer, nullptr, arrow_verts.data(), 3, nullptr, 0);

	switch (orientation)
	{
		using enum model::BlockOrientation;
	case LeftToRight:
	{
		arrow_verts[0].position = SDL_FPoint{ static_cast<float>(screen_bounds.x + 2), static_cast<float>(screen_bounds.y + 4) };
		arrow_verts[1].position = SDL_FPoint{ static_cast<float>(screen_bounds.x + 2), static_cast<float>(screen_bounds.y + screen_bounds.h - 4) };
		arrow_verts[2].position = SDL_FPoint{ static_cast<float>(screen_bounds.x + screen_bounds.w - 4), static_cast<float>(screen_bounds.y + screen_bounds.h / 2) };
		break;
	}
	case TopToBottom:
	{
		arrow_verts[0].position = SDL_FPoint{ static_cast<float>(screen_bounds.x + 4), static_cast<float>(screen_bounds.y + 2) };
		arrow_verts[1].position = SDL_FPoint{ static_cast<float>(screen_bounds.x + screen_bounds.w - 4), static_cast<float>(screen_bounds.y + 2) };
		arrow_verts[2].position = SDL_FPoint{ static_cast<float>(screen_bounds.x + screen_bounds.w / 2), static_cast<float>(screen_bounds.y + screen_bounds.h - 4) };
		break;
	}
	case RightToLeft:
	{
		arrow_verts[0].position = SDL_FPoint{ static_cast<float>(screen_bounds.x + screen_bounds.w - 2), static_cast<float>(screen_bounds.y + 4) };
		arrow_verts[1].position = SDL_FPoint{ static_cast<float>(screen_bounds.x + screen_bounds.w - 2), static_cast<float>(screen_bounds.y + screen_bounds.h - 4) };
		arrow_verts[2].position = SDL_FPoint{ static_cast<float>(screen_bounds.x + 4), static_cast<float>(screen_bounds.y + screen_bounds.h / 2) };
		break;
	}
	case BottomToTop:
	{
		arrow_verts[0].position = SDL_FPoint{ static_cast<float>(screen_bounds.x + 4), static_cast<float>(screen_bounds.y + screen_bounds.h - 2) };
		arrow_verts[1].position = SDL_FPoint{ static_cast<float>(screen_bounds.x + screen_bounds.w - 4), static_cast<float>(screen_bounds.y + screen_bounds.h - 2) };
		arrow_verts[2].position = SDL_FPoint{ static_cast<float>(screen_bounds.x + screen_bounds.w / 2), static_cast<float>(screen_bounds.y + 4) };
		break;
	}
	}
	for (auto&& vert : arrow_verts)
	{
		vert.color = inner_color_f;
	}
	SDL_RenderGeometry(renderer, nullptr, arrow_verts.data(), 3, nullptr, 0);

}

void node::GainBlockStyler::UpdateProperties(const model::BlockDataCRef& model)
{
	SetText(GetBlockGain(model.GetFunctionalData()));
}
