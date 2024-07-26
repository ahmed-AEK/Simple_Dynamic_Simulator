#include "ToolBar.hpp"

node::ToolBar::ToolBar(const SDL_Rect& rect, Scene* parent)
	:Widget(rect, parent)
{

}

node::ToolBar::~ToolBar()
{

}

void node::ToolBar::AddButton(std::unique_ptr<ToolBarButton> button, int position)
{

	if (position == -1)
	{
		m_buttons.push_back(std::move(button));
		return;
	}

	if (position < m_buttons.size())
	{
		m_buttons.insert(m_buttons.begin() + position, std::move(button));
	}
	else
	{
		m_buttons.push_back(std::move(button));
	}
	
}

void node::ToolBar::Draw(SDL_Renderer * renderer)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &GetRect());
	SDL_Rect inner_rect = GetRect();
	inner_rect.x += 2;
	inner_rect.y += 2;
	inner_rect.w -= 4;
	inner_rect.h -= 4;
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(renderer, &inner_rect);
	for (auto&& button : m_buttons)
	{
		button->Draw(renderer);
	}
}

node::Widget* node::ToolBar::OnGetInteractableAtPoint(const SDL_Point& point)
{
	for (auto&& button : m_buttons)
	{
		if (auto result = button->GetInteractableAtPoint(point))
		{
			return result;
		}
	}
	return Widget::OnGetInteractableAtPoint(point);
}

void node::ToolBar::OnSetRect(const SDL_Rect& rect)
{
	Widget::OnSetRect(rect);

	int position = 0;
	for (auto&& button : m_buttons)
	{
		button->SetRect({ position * (ToolBarButton::Hmargin + ToolBarButton::width) + ToolBarButton::Hmargin, 4, ToolBarButton::width, ToolBarButton::height });
		position++;
	}
}


node::ToolBarButton::ToolBarButton(const SDL_Rect& rect, Scene* parent)
	:Widget(rect, parent)
{
}

node::ToolBarButton::~ToolBarButton()
{
}

std::string_view node::ToolBarButton::GetName() noexcept
{
	return m_name;
}

void node::ToolBarButton::Draw(SDL_Renderer* renderer)
{
	SDL_Color color = b_hovered ? SDL_Color{ 255,150,0,255 } : SDL_Color{0, 0, 0, 255};
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	SDL_RenderFillRect(renderer, &GetRect());
	SDL_Rect inner_rect = GetRect();
	inner_rect.x += 2;
	inner_rect.y += 2;
	inner_rect.w -= 4;
	inner_rect.h -= 4;
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(renderer, &inner_rect);
}

void node::ToolBarButton::OnMouseOut()
{
	b_hovered = false;
}

void node::ToolBarButton::OnMouseIn()
{
	b_hovered = true;
}
