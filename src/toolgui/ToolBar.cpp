#include "ToolBar.hpp"
#include "Scene.hpp"
#include "Application.hpp"

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

	if (static_cast<size_t>(position) < m_buttons.size())
	{
		m_buttons.insert(m_buttons.begin() + position, std::move(button));
	}
	else
	{
		m_buttons.push_back(std::move(button));
	}
	
}

node::ToolBarButton* node::ToolBar::GetButton(const std::string& name)
{
	auto it = std::find_if(m_buttons.begin(), m_buttons.end(), [&](auto&& button) { return button->GetName() == name; });
	if (it != m_buttons.end())
	{
		return it->get();
	}
	return nullptr;
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


node::ToolBarButton::ToolBarButton(const SDL_Rect& rect, Scene* parent, std::string name)
	:Widget(rect, parent), m_name{std::move(name)}
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
	color = b_active ? SDL_Color{230, 230, 230, 255} : SDL_Color{ 255, 255, 255, 255 };
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	SDL_RenderFillRect(renderer, &inner_rect);

	if (!m_name_texture)
	{
		SDL_Color Black = { 50, 50, 50, 255 };
		auto surface = SDLSurface{ TTF_RenderText_Solid(GetScene()->GetApp()->getFont().get(), m_name.c_str(), Black)};
		m_name_texture = SDLTexture{ SDL_CreateTextureFromSurface(renderer, surface.get()) };
	}
	SDL_Rect text_rect{};
	SDL_QueryTexture(m_name_texture.get(), NULL, NULL, &text_rect.w, &text_rect.h);
	text_rect.x = inner_rect.x + inner_rect.w / 2 - text_rect.w / 2;
	text_rect.y = inner_rect.y + inner_rect.h / 2 - text_rect.h / 2;
	SDL_RenderCopy(renderer, m_name_texture.get(), NULL, &text_rect);
}

void node::ToolBarButton::SetActive(bool value)
{
	b_active = value;
}

void node::ToolBarButton::OnMouseOut()
{
	b_hovered = false;
	b_held_down = false;
}

void node::ToolBarButton::OnMouseIn()
{
	b_hovered = true;
}

void node::ToolBarButton::OnButonClicked()
{
	b_active = !b_active;
}

MI::ClickEvent node::ToolBarButton::OnLMBDown(const SDL_Point& current_mouse_point)
{
	UNUSED_PARAM(current_mouse_point);
	b_held_down = true;
	return MI::ClickEvent::CLICKED;
}

MI::ClickEvent node::ToolBarButton::OnLMBUp(const SDL_Point& current_mouse_point)
{
	UNUSED_PARAM(current_mouse_point);
	if (b_held_down)
	{
		OnButonClicked();
	}
	b_held_down = false;
	return MI::ClickEvent::CLICKED;
}
