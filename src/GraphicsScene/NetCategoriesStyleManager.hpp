#pragma once

#include "NodeModels/IdTypes.hpp"
#include <set>

namespace node
{

class NetCategoryStyle
{
public:
	NetCategoryStyle() = default;
	void SetCategory(const model::NetCategory& category) { m_category = category; }
	void SetColor(const SDL_Color& color) { m_color = color; }
	const SDL_Color& GetColor() const { return m_color; }
	const model::NetCategory& GetCategory() const { return m_category; }
private:
	model::NetCategory m_category{};
	SDL_Color m_color{};
};

class NetCategoriesStyleManager
{
public:
	NetCategoriesStyleManager() = default;
	std::shared_ptr<const node::NetCategoryStyle> SetStyle(const model::NetCategory& category, SDL_Color color);
	std::shared_ptr<const NetCategoryStyle> GetStyle(const model::NetCategory& category);
private:
	std::unordered_map<model::NetCategory, std::shared_ptr<NetCategoryStyle>> m_styles;
};

}