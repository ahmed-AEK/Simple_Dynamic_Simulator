#include "NetCategoriesStyleManager.hpp"

std::shared_ptr<const node::NetCategoryStyle> node::NetCategoriesStyleManager::SetStyle(const model::NetCategory& category, SDL_Color color)
{
	auto& ptr = m_styles[category];
	if (ptr)
	{
		ptr->SetColor(color);
	}
	else
	{
		ptr = std::make_shared<NetCategoryStyle>();
		ptr->SetCategory(category);
		ptr->SetColor(color);
	}
	return ptr;
}

std::shared_ptr<const node::NetCategoryStyle> node::NetCategoriesStyleManager::GetStyle(const model::NetCategory& category)
{
	auto& ptr = m_styles[category];
	if (!ptr)
	{
		ptr = std::make_shared<NetCategoryStyle>();
		ptr->SetCategory(category);
		ptr->SetColor(SDL_Color{255,0,0,0});
	}
	return ptr;
}
