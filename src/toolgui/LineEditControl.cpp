#include "LineEditControl.hpp"
#include "Scene.hpp"
#include "Application.hpp"

node::LineEditControl::LineEditControl(std::string initial_value, const SDL_Rect& rect, Scene* parent)
	:Widget{ rect,parent }, m_value{ std::move(initial_value) }, m_painter{ GetScene()->GetApp()->getFont().get() }
{
	SetFocusable(true);
	m_painter.SetText(m_value);
}

void node::LineEditControl::Draw(SDL_Renderer* renderer)
{
	SDL_Rect edit_box = GetRect();
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &edit_box);
	SDL_Rect inner_rect{ edit_box };
	inner_rect.x += 1;
	inner_rect.y += 1;
	inner_rect.w -= 2;
	inner_rect.h -= 2;
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(renderer, &inner_rect);
	{
		SDL_Color Black = { 50, 50, 50, 255 };
		SDL_Point text_start{ inner_rect.x, inner_rect.y };
		m_painter.Draw(renderer, text_start, Black);
	}
}

void node::LineEditControl::OnKeyPress(int32_t key)
{
	if (key == SDL_SCANCODE_BACKSPACE)
	{
		SDL_Log("BackSpace");
		if (m_value.size())
		{
			m_value.pop_back();
			m_painter.SetText(m_value);
		}
	}
	else if (key == SDL_SCANCODE_RETURN)
	{
		SDL_Log("Return");
	}
	else
	{
		SDL_Log("key %d", key);
	}
}

void node::LineEditControl::OnChar(int key)
{
	char keys[2]{ static_cast<char>(key), '\0' };
	SDL_Log("%s", keys);
	if (key < 128 && key >= 0)
	{
		m_value.push_back(static_cast<char>(key));
		m_painter.SetText(m_value);
	}
}
