#include "ToolBar.hpp"
#include "Scene.hpp"
#include "Application.hpp"
#include "SDL_Framework/Utility.hpp"

node::ToolBar::ToolBar(const SDL_Rect& rect, Scene* parent)
	:Widget(rect, parent)
{

}

static bool isButton(const node::ToolBar::ToolBarElement& element)
{
	return std::holds_alternative<std::unique_ptr<node::ToolBarButton>>(element);
}

static std::unique_ptr<node::ToolBarButton>& AsButton(node::ToolBar::ToolBarElement& element)
{
	return std::get<std::unique_ptr<node::ToolBarButton>>(element);
}

static node::ToolBar::ToolBarSeparator& AsSeparator(node::ToolBar::ToolBarElement& element)
{
	return std::get<node::ToolBar::ToolBarSeparator>(element);
}

node::ToolBar::~ToolBar()
{
}

void node::ToolBar::AddButton(std::unique_ptr<ToolBarButton> button, int position)
{
	if (position == -1)
	{
		m_buttons.push_back(std::move(button));
	}
	else if (static_cast<size_t>(position) < m_buttons.size())
	{
		m_buttons.insert(m_buttons.begin() + position, std::move(button));
	}
	else
	{
		m_buttons.push_back(std::move(button));
	}
	RepositionButtons();

}

void node::ToolBar::AddSeparator(int position)
{
	if (position == -1)
	{
		m_buttons.push_back(ToolBarSeparator{});
		return;
	}

	if (static_cast<size_t>(position) < m_buttons.size())
	{
		m_buttons.insert(m_buttons.begin() + position, std::move(ToolBarSeparator{}));
	}
	else
	{
		m_buttons.push_back(ToolBarSeparator{});
	}
}

node::ToolBarButton* node::ToolBar::GetButton(const std::string& name)
{
	auto it = std::find_if(m_buttons.begin(), m_buttons.end(), [&](auto&& button) { 
		if (isButton(button))
		{
			return AsButton(button)->GetName() == name;
		}
		return false;
		 });
	if (it != m_buttons.end())
	{
		return  AsButton(*it).get();
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
		if (isButton(button))
		{
			AsButton(button)->Draw(renderer);
		}
		else
		{
			SDL_Rect separator_rect{ AsSeparator(button).position_x, inner_rect.y + ToolBarSeparator::VMargin, 
				ToolBarSeparator::width, inner_rect.h - 2 * ToolBarSeparator::VMargin };
			SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
			SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
			SDL_RenderFillRect(renderer, &separator_rect);
		}
	}
}

node::Widget* node::ToolBar::OnGetInteractableAtPoint(const SDL_Point& point)
{
	for (auto&& button : m_buttons)
	{
		if (!isButton(button))
		{
			continue;
		}
		if (auto result = AsButton(button)->GetInteractableAtPoint(point))
		{
			return result;
		}
	}
	return Widget::OnGetInteractableAtPoint(point);
}

void node::ToolBar::OnSetRect(const SDL_Rect& rect)
{
	Widget::OnSetRect(rect);
	RepositionButtons();
	
}

void node::ToolBar::RepositionButtons()
{
	int position_x = ToolBarButton::Hmargin + GetRect().x;
	int position_y = GetRect().y;
	for (auto&& button : m_buttons)
	{
		if (isButton(button))
		{
			AsButton(button)->SetRect({ position_x, position_y + 4, ToolBarButton::width, ToolBarButton::height });
			position_x += ToolBarButton::Hmargin + ToolBarButton::width;
		}
		else
		{
			AsSeparator(button).position_x = position_x;
			position_x += ToolBarSeparator::width + ToolBarButton::Hmargin;
		}

	}
}


node::ToolBarButton::ToolBarButton(const SDL_Rect& rect, Scene* parent, std::string name)
	:Widget(rect, parent), m_name{std::move(name)}, 
	m_painter_outer{std::make_unique<RoundRectPainter>()},
	m_painter_inner{ std::make_unique<RoundRectPainter>() }
{
}

node::ToolBarButton::~ToolBarButton()
{
}

void node::ToolBarButton::Draw(SDL_Renderer* renderer)
{
	const int thickness = 2;
	{
		SDL_Color color_outer = b_hovered ? SDL_Color{ 255,150,0,255 } : SDL_Color{ 180, 180, 180, 255 };
		const int radius = 10;
		SDL_Color inactive_color = b_held_down ? SDL_Color{ 230, 230, 230, 255 } : SDL_Color{ 255, 255, 255, 255 };
		SDL_Color color_inner = IsActive() ? SDL_Color{230, 230, 230, 255} : inactive_color;
		ThickFilledRoundRect(renderer, GetRect(), radius, thickness, color_outer, color_inner, 
			*m_painter_outer, *m_painter_inner);
	}


	SDL_Rect inner_rect = GetRect();
	inner_rect.x += thickness;
	inner_rect.y += thickness;
	inner_rect.w -= 2 * thickness;
	inner_rect.h -= 2 * thickness;
	if (!m_name_texture)
	{
		SDL_Color Black = { 50, 50, 50, 255 };
		auto surface = SDLSurface{ TTF_RenderText_Blended(GetScene()->GetApp()->getFont().get(), m_name.c_str(), Black)};
		m_name_texture = SDLTexture{ SDL_CreateTextureFromSurface(renderer, surface.get()) };
	}
	SDL_Rect text_rect{};
	SDL_QueryTexture(m_name_texture.get(), NULL, NULL, &text_rect.w, &text_rect.h);
	text_rect.x = inner_rect.x + inner_rect.w / 2 - text_rect.w / 2;
	text_rect.y = inner_rect.y + inner_rect.h / 2 - text_rect.h / 2;
	SDL_RenderCopy(renderer, m_name_texture.get(), NULL, &text_rect);
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

node::ToolBarCommandButton::ToolBarCommandButton(const SDL_Rect& rect, Scene* parent, 
	std::string name, std::function<void()> func, std::function<bool()> Active)
	:ToolBarButton{rect, parent, name}, m_action{std::move(func)}, m_isActive{Active}
{
}

void node::ToolBarCommandButton::OnButonClicked()
{
	if (!m_isActive())
	{
		m_action();
	}
}