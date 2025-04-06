#include "LineEditControl.hpp"
#include "Scene.hpp"
#include "Application.hpp"

node::LineEditControl::LineEditControl(std::string initial_value, const WidgetSize& size, TTF_Font* font, Widget* parent)
	:Widget{ size, parent }, m_value{ std::move(initial_value) }, 
	m_painter{ font }, m_cursor_position{m_value.size()}
{
	SetFocusable(true);
	m_painter.SetText(m_value);
	ReCalculateCursorPixelPosition();
}

void node::LineEditControl::OnDraw(SDL::Renderer& renderer)
{
	const SDL_FRect edit_box = GetSize().ToRect();
	ThickFilledRoundRect(renderer, edit_box, 8, 1, { 0,0,0,255 }, { 255,255,255,255 }, m_outer_painter, m_inner_painter);

	SDL_FRect inner_rect{ edit_box };
	inner_rect.x += 1;
	inner_rect.y += 1;
	inner_rect.w -= 2;
	inner_rect.h -= 2;
	{
		SDL_Color Black = { 50, 50, 50, 255 };
		auto text_Rect = m_painter.GetRect(renderer, Black);
		SDL_FPoint text_start{ inner_rect.x + H_Margin, inner_rect.y + inner_rect.h - 5 - text_Rect.h };
		m_painter.Draw(renderer, text_start, Black);
	}
	if (m_focused)
	{
		SDL_FRect cursor_rect{ inner_rect.x + H_Margin + m_cursor_pixel_position, inner_rect.y + 4, 2, inner_rect.h - 8 };
		SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
		SDL_RenderFillRect(renderer, &cursor_rect);
	}
}

node::LineEditControl::~LineEditControl()
{
	if (m_focused)
	{
		GetApp()->StopTextInput();
	}
}

MI::ClickEvent node::LineEditControl::OnLMBDown(MouseButtonEvent& e)
{
	if (!m_value.size())
	{
		// edit empty, nothing to do
		return MI::ClickEvent::CLICKED;
	}
	SDL_FPoint current_mouse_point{ e.point() };
	int click_pos_x = static_cast<int>(current_mouse_point.x - H_Margin);
	int extent = 0;
	size_t count = 0;
	if (!TTF_MeasureString(m_painter.GetFont(), m_value.c_str(), m_value.size(), click_pos_x, &extent, &count))
	{
		// failed for some reason
		return MI::ClickEvent::CLICKED;
	}
	size_t new_cursor_pos = count;

	if (count && static_cast<size_t>(count) < m_value.size())
	{
		// compare it with next character too
		int w = 0, h = 0;
		TTF_GetStringSize(m_painter.GetFont(), m_value.c_str(), new_cursor_pos + 1, &w, &h);
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

bool node::LineEditControl::OnKeyPress(KeyboardEvent& e)
{
	if (e.e.type != SDL_EVENT_KEY_DOWN)
	{
		return false;
	}
	int key = e.e.scancode;
	if (key == SDL_SCANCODE_BACKSPACE)
	{
		m_logger.LogDebug("BackSpace");
		if (m_value.size() && m_cursor_position > 0)
		{
			m_value.erase(m_value.begin() + m_cursor_position - 1);
			m_painter.SetText(m_value);
			m_cursor_position--;
			ReCalculateCursorPixelPosition();
		}
		return true;
	}
	else if (key == SDL_SCANCODE_DELETE)
	{
		if (m_cursor_position < m_value.size())
		{
			m_value.erase(m_value.begin() + m_cursor_position);
			m_painter.SetText(m_value);
		}
		return true;
	}
	else if (key == SDL_SCANCODE_LEFT)
	{
		if (m_cursor_position > 0)
		{
			m_cursor_position--;
			ReCalculateCursorPixelPosition();
		}
		return true;
	}
	else if (key == SDL_SCANCODE_RIGHT)
	{
		if (m_cursor_position < m_value.size())
		{
			m_cursor_position++;
			ReCalculateCursorPixelPosition();
		}
		return true;
	}
	else if (key == SDL_SCANCODE_RETURN || key == SDL_SCANCODE_KP_ENTER)
	{
		m_logger.LogDebug("Return");
	}
	else
	{
		m_logger.LogDebug("key {}", key);
	}
	return false;
}

void node::LineEditControl::OnKeyboardFocusIn()
{
	m_focused = true;
	auto screen_position = GetGlobalPosition();
	auto rect = ToRect(GetSize().ToRectWithOrigin(screen_position));
	GetApp()->StartTextInput(rect);
}

void node::LineEditControl::OnKeyboardFocusOut()
{
	m_focused = false;
	GetApp()->StopTextInput();
}

void node::LineEditControl::ReCalculateCursorPixelPosition()
{
	int w = 0, h = 0;
	if (m_cursor_position == 0)
	{
		m_cursor_pixel_position = 0;
	}
	else
	{
		if (TTF_GetStringSize(m_painter.GetFont(), m_value.c_str(), m_cursor_position, &w, &h))
		{
			m_cursor_pixel_position = w;
		}
	}
}

bool node::LineEditControl::OnChar(TextInputEvent& e)
{
	int key = e.e.text[0];
	m_logger.LogDebug("{}", key);
	if (key < 128 && key >= 0)
	{
		m_value.insert(m_value.begin() + m_cursor_position, static_cast<char>(key));
		m_painter.SetText(m_value);
		m_cursor_position++;
		ReCalculateCursorPixelPosition();
		return true;
	}
	return false;
}
