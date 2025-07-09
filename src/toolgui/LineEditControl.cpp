#include "LineEditControl.hpp"
#include "Scene.hpp"
#include "Application.hpp"

node::LineEditControl::LineEditControl(std::string initial_value, const WidgetSize& size, TTF_Font* font, Widget* parent)
	:Widget{ size, parent }, m_value{ std::move(initial_value) }, 
	m_painter{ font }, m_cursor_position{m_value.size()}
{
	SetFocusable(true);
	m_painter.SetText(m_value);
	m_painter.SetWidth(static_cast<size_t>(GetTextArea().w));
	ReCalculateCursorsPixelPosition();
}

void node::LineEditControl::OnDraw(SDL::Renderer& renderer)
{
	const SDL_FRect edit_box = GetSize().ToRect();
	const SDL_Color text_color = renderer.GetColor(ColorRole::text_normal);
	const SDL_Color outline_color = renderer.GetColor(ColorRole::frame_outline);
	const SDL_Color background_color = renderer.GetColor(ColorRole::frame_background);
	ThickFilledRoundRect(renderer, edit_box, 8, 1, outline_color, background_color, m_outer_painter, m_inner_painter);

	
	DrawSelectionRect(renderer);
	SDL_FRect inner_rect = GetTextArea();
	{
		auto text_height = static_cast<float>(m_painter.GetHeight());
		SDL_FPoint text_start{ inner_rect.x, inner_rect.y + (inner_rect.h - text_height) * 0.5f};
		m_painter.Draw(renderer, text_start, text_color);
	}
	if (m_focused)
	{
		SDL_FRect cursor_rect{ inner_rect.x + m_cursor_pixel_position - m_painter.GetPixelOffset(), 
			inner_rect.y + 4, 2, inner_rect.h - 8};
		SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, text_color.a);
		SDL_RenderFillRect(renderer, &cursor_rect);
	}
}

void node::LineEditControl::DrawSelectionRect(SDL::Renderer& renderer)
{
	if (!m_selection_active)
	{
		return;
	}
	SDL_FRect inner_rect = GetTextArea();
	size_t pixel_inner_start = m_painter.GetPixelOffset();
	size_t pixel_inner_end = static_cast<size_t>(m_painter.GetPixelOffset() + inner_rect.w);
	const SDL_Color Blue = renderer.GetColor(ColorRole::blue_select);
	auto start_pos = std::min(m_selection_anchor_pixel_pos, m_selection_start_pixel_pos);
	auto end_pos = std::max(m_selection_anchor_pixel_pos, m_selection_start_pixel_pos);
	if (start_pos < pixel_inner_start && end_pos < pixel_inner_start)
	{
		return;
	}
	if (start_pos > pixel_inner_end && end_pos > pixel_inner_end)
	{
		return;
	}
	if (start_pos < pixel_inner_start)
	{
		start_pos = pixel_inner_start;
	}
	if (end_pos > pixel_inner_end)
	{
		end_pos = pixel_inner_end;
	}
	SDL_FRect selection_rect{ start_pos - pixel_inner_start + inner_rect.x, inner_rect.y, static_cast<float>(end_pos - start_pos), inner_rect.h };
	SDL_SetRenderDrawColor(renderer, Blue.r, Blue.g, Blue.b, Blue.a);
	SDL_RenderFillRect(renderer, &selection_rect);
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
	
	if (e.e.clicks == 2)
	{
		m_selection_active = true;
		m_selection_start = 0;
		m_selection_anchor = m_value.size();
		m_cursor_position = m_value.size();
		ReCalculateCursorsPixelPosition();
		return MI::ClickEvent::CLICKED;
	}

	auto clicked_character = GetCharacterAtScreenPosition(current_mouse_point.x);
	m_cursor_position = clicked_character.character_offset;
	m_cursor_pixel_position = clicked_character.pixel_offset;
	m_selection_start = m_cursor_position;
	m_selection_start_pixel_pos = m_cursor_pixel_position;
	m_selection_active = false;
	m_dragging = true;
	UpdateOffset();

	return MI::ClickEvent::CAPTURE_START;
}

MI::ClickEvent node::LineEditControl::OnLMBUp(MouseButtonEvent& e)
{
	if (!m_dragging)
	{
		return MI::ClickEvent::CAPTURE_END;
	}
	m_dragging = false;

	if (!m_value.size())
	{
		return MI::ClickEvent::CAPTURE_END;
	}

	SDL_FPoint current_mouse_point{ e.point() };
	auto clicked_character = GetCharacterAtScreenPosition(current_mouse_point.x);
	m_cursor_position = clicked_character.character_offset;
	m_cursor_pixel_position = clicked_character.pixel_offset;
	m_selection_anchor = m_cursor_position;
	m_selection_anchor_pixel_pos = m_cursor_pixel_position;
	m_selection_active = m_selection_anchor != m_selection_start;
	UpdateOffset();

	return MI::ClickEvent::CAPTURE_END;
}

void node::LineEditControl::OnMouseMove(MouseHoverEvent& e)
{
	if (!m_dragging)
	{
		return;
	}

	auto clicked_character = GetCharacterAtScreenPosition(e.point().x);
	m_cursor_position = clicked_character.character_offset;
	m_cursor_pixel_position = clicked_character.pixel_offset;
	m_selection_anchor = m_cursor_position;
	m_selection_anchor_pixel_pos = m_cursor_pixel_position;
	m_selection_active = m_selection_anchor != m_selection_start;
	UpdateOffset();
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
		if (DeleteSelection())
		{
			ReCalculateCursorsPixelPosition();
		}
		else if (m_value.size() && m_cursor_position > 0)
		{
			m_value.erase(m_value.begin() + m_cursor_position - 1);
			m_painter.SetText(m_value);
			m_cursor_position--;
			ReCalculateCursorsPixelPosition();
		}
		return true;
	}
	else if (key == SDL_SCANCODE_DELETE)
	{
		if (DeleteSelection())
		{
			ReCalculateCursorsPixelPosition();
		}
		else if (m_cursor_position < m_value.size())
		{
			m_value.erase(m_value.begin() + m_cursor_position);
			m_painter.SetText(m_value);
		}
		return true;
	}
	else if (key == SDL_SCANCODE_LEFT)
	{
		// handle selection
		if (e.e.mod & SDL_KMOD_SHIFT)
		{
			if (!m_selection_active)
			{
				m_selection_active = true;
				m_selection_start = m_cursor_position;
			}
		}
		else
		{
			m_selection_active = false;
		}

		// handle motion
		if (m_cursor_position > 0)
		{
			if (!(e.e.mod & SDL_KMOD_CTRL))
			{
				m_cursor_position--;
			}
			else
			{
				while (m_cursor_position > 0 && m_value[m_cursor_position - 1] == ' ')
				{
					m_cursor_position--;
				}
				while (m_cursor_position > 0 && m_value[m_cursor_position - 1] != ' ')
				{
					m_cursor_position--;
				}
			}
			if (e.e.mod & SDL_KMOD_SHIFT)
			{
				m_selection_anchor = m_cursor_position;
				m_selection_active = m_selection_start != m_selection_anchor;
			}
			ReCalculateCursorsPixelPosition();
		}
		return true;
	}
	else if (key == SDL_SCANCODE_RIGHT)
	{
		// handle selection
		if (e.e.mod & SDL_KMOD_SHIFT)
		{
			if (!m_selection_active)
			{
				m_selection_active = true;
				m_selection_start = m_cursor_position;
			}
		}
		else
		{
			m_selection_active = false;
		}

		// handle motion
		if (m_cursor_position < m_value.size())
		{
			if (!(e.e.mod & SDL_KMOD_CTRL))
			{
				m_cursor_position++;
			}
			else
			{
				while (m_cursor_position < m_value.size() && m_value[m_cursor_position] != ' ')
				{
					m_cursor_position++;
				}
				while (m_cursor_position < m_value.size() && m_value[m_cursor_position] == ' ')
				{
					m_cursor_position++;
				}
			}
			if (e.e.mod & SDL_KMOD_SHIFT)
			{
				m_selection_anchor = m_cursor_position;
				m_selection_active = m_selection_start != m_selection_anchor;
			}
			ReCalculateCursorsPixelPosition();
		}
		return true;
	}
	else if (key == SDL_SCANCODE_RETURN || key == SDL_SCANCODE_KP_ENTER)
	{
		m_logger.LogDebug("Return");
	}
	else if (key == SDL_SCANCODE_C && e.e.mod & SDL_KMOD_LCTRL)
	{
		if (!m_selection_active)
		{
			return false;
		}
		return CopySelection();
	}
	else if (key == SDL_SCANCODE_V && e.e.mod & SDL_KMOD_LCTRL)
	{
		auto* app = GetApp();
		if (!app)
		{
			return false;
		}
		DeleteSelection();
		auto clipboard_text = app->GetClipboardText();
		auto view = clipboard_text.view();
		if (view.size())
		{
			m_value.insert(m_value.begin() + m_cursor_position, view.begin(), view.end());
			m_painter.SetText(m_value);
			m_cursor_position += clipboard_text.view().size();
		}
		ReCalculateCursorsPixelPosition();
	}
	else if (key == SDL_SCANCODE_X && e.e.mod & SDL_KMOD_LCTRL)
	{
		if (!m_selection_active)
		{
			return false;
		}
		auto* app = GetApp();
		if (!app)
		{
			return false;
		}

		if (!CopySelection())
		{
			return false;
		}

		if (!DeleteSelection())
		{
			return false;
		}
		ReCalculateCursorsPixelPosition();
		return true;
	}
	else if (key == SDL_SCANCODE_A && e.e.mod & SDL_KMOD_LCTRL)
	{
		if (!m_value.size())
		{
			return false;
		}
		m_selection_active = true;
		m_selection_start = 0;
		m_selection_anchor = m_value.size();
		m_cursor_position = m_value.size();
		ReCalculateCursorsPixelPosition();
		return true;
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
	auto text_rect = GetTextArea();
	text_rect.x += screen_position.x;
	text_rect.y += screen_position.y;
	GetApp()->StartTextInput(ToRect(text_rect));
}

void node::LineEditControl::OnKeyboardFocusOut()
{
	m_selection_active = false;
	m_focused = false;
	GetApp()->StopTextInput();
}

void node::LineEditControl::OnSetSize(const WidgetSize& size)
{
	Widget::OnSetSize(size);
	m_painter.SetWidth(static_cast<size_t>(GetTextArea().w));
	ReCalculateCursorsPixelPosition();
}

void node::LineEditControl::ReCalculateCursorsPixelPosition()
{
	ReCalculateMainCursorPixelPosition();
	if (m_selection_active)
	{
		ReCalculateSelectionCursorsPixelPosition(m_selection_anchor, m_selection_anchor_pixel_pos);
		ReCalculateSelectionCursorsPixelPosition(m_selection_start, m_selection_start_pixel_pos);
	}
}

void node::LineEditControl::ReCalculateMainCursorPixelPosition()
{
	if (m_cursor_position == 0)
	{
		m_cursor_pixel_position = 0;
		m_painter.SetPixelOffset(0);
		return;
	}

	int w = 0, h = 0;
	if (!TTF_GetStringSize(m_painter.GetFont(), m_value.c_str(), m_cursor_position, &w, &h))
	{
		return;
	}

	m_cursor_pixel_position = w;
	UpdateOffset();
}

void node::LineEditControl::ReCalculateSelectionCursorsPixelPosition(const size_t position, size_t& pixel_position)
{
	if (position == 0)
	{
		pixel_position = 0;
		return;
	}

	int w = 0, h = 0;
	if (!TTF_GetStringSize(m_painter.GetFont(), m_value.c_str(), position, &w, &h))
	{
		pixel_position = 0;
		return;
	}

	pixel_position = w;
}

void node::LineEditControl::UpdateOffset()
{
	size_t width = static_cast<size_t>(GetTextArea().w);
	if (m_cursor_pixel_position > min_cursor_distance_from_side && width > min_cursor_distance_from_side &&
		m_cursor_pixel_position > width + m_painter.GetPixelOffset() - min_cursor_distance_from_side)
	{
		m_painter.SetPixelOffset(m_cursor_pixel_position - width + min_cursor_distance_from_side);
	}
	else if (width > min_cursor_distance_from_side && m_painter.GetPixelOffset() > m_cursor_pixel_position)
	{
		if (m_cursor_pixel_position > min_cursor_distance_from_side)
		{
			m_painter.SetPixelOffset(m_cursor_pixel_position - min_cursor_distance_from_side);
		}
		else
		{
			m_painter.SetPixelOffset(static_cast<size_t>(0));
		}
	}
}

node::LineEditControl::CharacterPosition node::LineEditControl::GetCharacterAtScreenPosition(float x)
{
	auto text_rect = GetTextArea();
	if (x <= text_rect.x)
	{
		return { 0,0 };
	}
	int click_pos_x = static_cast<int>(x - text_rect.x + m_painter.GetPixelOffset());
	int extent = 0;
	size_t count = 0;
	if (!TTF_MeasureString(m_painter.GetFont(), m_value.c_str(), m_value.size(), click_pos_x, &extent, &count))
	{
		// failed for some reason
		return { 0,0 };
	}
	size_t new_cursor_pos = count;
	size_t pixel_offset = extent;
	if (count && static_cast<size_t>(count) < m_value.size())
	{
		// compare it with next character too
		int w = 0, h = 0;
		TTF_GetStringSize(m_painter.GetFont(), m_value.c_str(), new_cursor_pos + 1, &w, &h);
		if (click_pos_x - extent > w - click_pos_x)
		{
			new_cursor_pos++;
			pixel_offset = w;
		}
	}
	return { new_cursor_pos, pixel_offset };
}

SDL_FRect node::LineEditControl::GetTextArea() const
{
	SDL_FRect inner_rect = GetSize().ToRect();
	inner_rect.x += 1 + H_Margin;
	inner_rect.y += 1;
	inner_rect.w -= 2 + 2 * H_Margin;
	inner_rect.h -= 2;
	return inner_rect;
}

bool node::LineEditControl::DeleteSelection()
{
	if (!m_selection_active)
	{
		return false;
	}
	m_selection_active = false;
	auto selection_start_pos = std::min(m_selection_start, m_selection_anchor);
	auto selection_end_pos = std::max(m_selection_start, m_selection_anchor);
	m_value.erase(m_value.begin() + selection_start_pos, m_value.begin() + selection_end_pos);
	m_painter.SetText(m_value);
	m_cursor_position = selection_start_pos;
	return true;

}

bool node::LineEditControl::CopySelection()
{
	auto* app = GetApp();
	if (!app)
	{
		return false;
	}
	auto start_pos = std::min(m_selection_anchor, m_selection_start);
	auto end_pos = std::max(m_selection_anchor, m_selection_start);
	assert(start_pos <= m_value.size());
	assert(end_pos <= m_value.size());
	std::string selected_part{ m_value.begin() + start_pos, m_value.begin() + end_pos };
	return app->SetClipboardText(selected_part);;
}

bool node::LineEditControl::OnChar(TextInputEvent& e)
{
	int key = e.e.text[0];
	m_logger.LogDebug("{}", key);
	if (key < 128 && key >= 0)
	{
		DeleteSelection();
		m_value.insert(m_value.begin() + m_cursor_position, static_cast<char>(key));
		m_painter.SetText(m_value);
		m_cursor_position++;
		ReCalculateCursorsPixelPosition();
		return true;
	}
	return false;
}
