#include "LineEditControl.hpp"
#include "Scene.hpp"
#include "Application.hpp"

node::LineEditControl::LineEditControl(std::string initial_value, const SDL_Rect& rect, Scene* parent)
	:Widget{ rect,parent }, m_value{ std::move(initial_value) }, m_painter{ GetScene()->GetApp()->getFont().get() }, m_cursor_position{m_value.size()}
{
	SetFocusable(true);
	m_painter.SetText(m_value);
	ReCalculateCursorPixelPosition();
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
	if (m_focused)
	{
		SDL_Rect cursor_rect{ inner_rect.x + m_cursor_pixel_position, inner_rect.y + 4, 2, inner_rect.h - 8 };
		SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
		SDL_RenderFillRect(renderer, &cursor_rect);
	}
}

MI::ClickEvent node::LineEditControl::OnLMBDown(const SDL_Point& current_mouse_point)
{
	if (!m_value.size())
	{
		// edit empty, nothing to do
		return MI::ClickEvent::CLICKED;
	}

	int click_pos_x = current_mouse_point.x - GetRect().x - 1;
	int extent = 0;
	int count = 0;
	if (TTF_MeasureText(m_painter.GetFont(), m_value.c_str(), click_pos_x, &extent, &count))
	{
		// failed for some reason
		return MI::ClickEvent::CLICKED;
	}
	size_t new_cursor_pos = count;

	if (count && static_cast<size_t>(count) < m_value.size())
	{
		// compare it with next character too
		int w = 0, h = 0;
		std::array<char, 256> buffer;
		assert(buffer.size() > m_value.size());
		std::copy(m_value.begin(), m_value.begin() + new_cursor_pos + 1, buffer.begin());
		buffer[new_cursor_pos + 1] = '\0';
		TTF_SizeText(m_painter.GetFont(), buffer.data(), &w, &h);
		if (click_pos_x - extent > w - click_pos_x)
		{
			new_cursor_pos++;
		}
	}
	if (new_cursor_pos != m_cursor_position)
	{
		m_cursor_position = new_cursor_pos;
		ReCalculateCursorPixelPosition();
	}
	return MI::ClickEvent::CLICKED;
}

void node::LineEditControl::OnKeyPress(int32_t key)
{
	if (key == SDL_SCANCODE_BACKSPACE)
	{
		SDL_Log("BackSpace");
		if (m_value.size() && m_cursor_position > 0)
		{
			m_value.erase(m_value.begin() + m_cursor_position - 1);
			m_painter.SetText(m_value);
			m_cursor_position--;
			ReCalculateCursorPixelPosition();
		}
	}
	else if (key == SDL_SCANCODE_DELETE)
	{
		if (m_cursor_position < m_value.size())
		{
			m_value.erase(m_value.begin() + m_cursor_position);
			m_painter.SetText(m_value);
		}
	}
	else if (key == SDL_SCANCODE_LEFT)
	{
		if (m_cursor_position > 0)
		{
			m_cursor_position--;
			ReCalculateCursorPixelPosition();
		}
	}
	else if (key == SDL_SCANCODE_RIGHT)
	{
		if (m_cursor_position < m_value.size())
		{
			m_cursor_position++;
			ReCalculateCursorPixelPosition();
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

void node::LineEditControl::OnKeyboardFocusIn()
{
	m_focused = true;
}

void node::LineEditControl::OnKeyboardFocusOut()
{
	m_focused = false;
}

void node::LineEditControl::ReCalculateCursorPixelPosition()
{
	int w = 0, h = 0;
	std::array<char, 256> buffer;
	assert(buffer.size() > m_value.size());
	std::copy(m_value.begin(), m_value.begin() + m_cursor_position, buffer.begin());
	buffer[m_cursor_position] = '\0';

	if (!TTF_SizeText(m_painter.GetFont(), buffer.data(), &w, &h))
	{
		m_cursor_pixel_position = w;
	}
}

void node::LineEditControl::OnChar(int key)
{
	SDL_Log("%d", key);
	if (key < 128 && key >= 0)
	{
		m_value.insert(m_value.begin() + m_cursor_position, static_cast<char>(key));
		m_painter.SetText(m_value);
		m_cursor_position++;
		ReCalculateCursorPixelPosition();
	}
}
