#include "MultiLineEditControl.hpp"
#include "Scene.hpp"
#include "Application.hpp"

node::MultiLineEditControl::MultiLineEditControl(const WidgetSize& size, TTF_Font* font, Widget* parent)
	:ScrollViewBase{ size, parent }, m_font{font}
{
	SetFocusable(true);
	SetScrollStrength(1);
}

void node::MultiLineEditControl::OnDraw(SDL::Renderer& renderer)
{
	const SDL_FRect edit_box = GetSize().ToRect();
	const SDL_Color outline_color = renderer.GetColor(ColorRole::frame_outline);
	const SDL_Color background_color = renderer.GetColor(ColorRole::frame_background);
	ThickFilledRoundRect(renderer, edit_box, 8, 1, outline_color, background_color, m_outer_painter, m_inner_painter);
	if (m_cursor_dirty)
	{
		m_cursor_dirty = false;
		UpdateCursorsPixelPosition();
	}
	DrawText(renderer);
	if (m_focused)
	{
		DrawCursor(renderer);
	}
}

void node::MultiLineEditControl::SetText(std::vector<std::string> text)
{
	m_text = std::move(text);
	m_painters.clear();
	m_cursor.position = { 0,0 };
	m_cursor_dirty = true;
	UpdateScrollBar();
}

node::MultiLineEditControl::~MultiLineEditControl()
{
	if (m_focused)
	{
		GetApp()->StopTextInput();
	}
}

MI::ClickEvent node::MultiLineEditControl::OnLMBDown(MouseButtonEvent& e)
{
	UNUSED_PARAM(e);
	auto clicked_character = GetCharacterAtScreenPosition(e.point());
	if (e.e.clicks == 3)
	{
		if (m_text.size() <= clicked_character.character_row)
		{
			m_cursor.position = { clicked_character.character_row, clicked_character.character_column };
			m_cursor_dirty = true;
			m_selection_active = false;
			return MI::ClickEvent::CLICKED;
		}
		m_selection_start.position = { clicked_character.character_row, 0 };
		m_selection_anchor.position = { clicked_character.character_row, m_text[clicked_character.character_row].size() };
		m_cursor.position = m_selection_anchor.position;
		m_cursor_dirty = true;
		m_selection_active = m_selection_anchor.position != m_selection_start.position;
		m_logger.LogDebug("cursor position: {}, {}", m_cursor.position.row, m_cursor.position.column);
		return MI::ClickEvent::CLICKED;
	}

	m_cursor.position = { clicked_character.character_row, clicked_character.character_column };
	m_cursor_dirty = true;
	m_selection_active = false;
	m_dragging = true;
	m_selection_start.position = m_cursor.position;
	m_logger.LogDebug("cursor position: {}, {}", m_cursor.position.row, m_cursor.position.column);
	return MI::ClickEvent::CAPTURE_START;
}


MI::ClickEvent node::MultiLineEditControl::OnLMBUp(MouseButtonEvent& e)
{
	if (!m_dragging)
	{
		return MI::ClickEvent::CAPTURE_END;
	}
	m_dragging = false;

	auto clicked_character = GetCharacterAtScreenPosition(e.point());

	m_cursor.position = { clicked_character.character_row, clicked_character.character_column };
	m_cursor_dirty = true;
	m_selection_anchor.position = m_cursor.position;
	m_selection_active = m_selection_anchor.position != m_selection_start.position;
	m_logger.LogDebug("cursor position: {}, {}", m_cursor.position.row, m_cursor.position.column);
	UpdateOffset();

	return MI::ClickEvent::CAPTURE_END;
}

void node::MultiLineEditControl::OnMouseMove(MouseHoverEvent& e)
{
	if (!m_dragging)
	{
		return;
	}

	auto clicked_character = GetCharacterAtScreenPosition(e.point());
	m_cursor.position = { clicked_character.character_row, clicked_character.character_column };
	m_cursor_dirty = true;
	m_selection_anchor.position = m_cursor.position;
	m_selection_active = m_selection_anchor.position != m_selection_start.position;
	UpdateOffset();
}
bool node::MultiLineEditControl::OnChar(TextInputEvent& e)
{
	int key = e.e.text[0];
	m_logger.LogDebug("{}", key);
	DeleteSelection();
	if (m_text.size() <= m_cursor.position.row)
	{
		while (m_text.size() <= m_cursor.position.row)
		{
			m_text.push_back({});
		}
		UpdateScrollBar();
	}
	auto& row = m_text[m_cursor.position.row];
	row.insert(row.begin() + m_cursor.position.column, static_cast<char>(key));
	m_cursor.position.column++;
	m_cursor_dirty = true;
	return true;
}

bool node::MultiLineEditControl::OnKeyPress(KeyboardEvent& e)
{
	if (e.e.type != SDL_EVENT_KEY_DOWN)
	{
		return false;
	}
	int key = e.e.scancode;
	m_logger.LogDebug("{} key", key);
	if (key == SDL_SCANCODE_BACKSPACE)
	{
		m_logger.LogDebug("BackSpace");
		if (DeleteSelection())
		{
			m_cursor_dirty = true;
			return true;
		}
		if (m_text.size() <= m_cursor.position.row)
		{
			if (m_cursor.position.row == 0)
			{
				// do nothing
			}
			else if (m_text.size() && m_cursor.position.row == m_text.size())
			{
				m_cursor.position.row--;
				auto& row = m_text[m_cursor.position.row];
				m_cursor.position.column = row.size();
				m_cursor_dirty = true;
			}
			else
			{
				m_cursor.position.row--;
				m_cursor_dirty = true;
			}
			return true;
		}
		auto& row = m_text[m_cursor.position.row];
		if (m_cursor.position.column == 0)
		{
			if (m_cursor.position.row == 0)
			{
				return true;
			}
			m_cursor.position.row--;
			m_cursor.position.column = m_text[m_cursor.position.row].size();
			m_text[m_cursor.position.row].insert(m_text[m_cursor.position.row].end(),
				m_text[m_cursor.position.row + 1].begin(), m_text[m_cursor.position.row + 1].end());
			m_text.erase(m_text.begin() + m_cursor.position.row + 1);
			UpdateScrollBar();
			m_cursor_dirty = true;
			return true;
		}
		assert(m_cursor.position.column <= row.size());
		if (m_cursor.position.column > 0)
		{
			row.erase(row.begin() + m_cursor.position.column - 1);
			m_cursor.position.column--;
			m_cursor_dirty = true;
		}
		return true;
	}
	else if (key == SDL_SCANCODE_DELETE)
	{
		m_logger.LogDebug("Delete");
		if (DeleteSelection())
		{
			m_cursor_dirty = true;
			return true;
		}
		if (m_text.size() <= m_cursor.position.row)
		{
			return true;
		}
		auto& row = m_text[m_cursor.position.row];
		if (m_cursor.position.column == row.size())
		{
			if (m_cursor.position.row == m_text.size() - 1)
			{
				return true;
			}
			m_text[m_cursor.position.row].insert(m_text[m_cursor.position.row].end(),
				m_text[m_cursor.position.row + 1].begin(), m_text[m_cursor.position.row + 1].end());
			m_text.erase(m_text.begin() + m_cursor.position.row + 1);
			UpdateScrollBar();
			return true;
		}

		assert(m_cursor.position.column < row.size());
		row.erase(row.begin() + m_cursor.position.column);
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
				m_selection_start = m_cursor;
			}
		}
		else
		{
			m_selection_active = false;
		}

		// handle motion
		if (m_cursor.position.column > 0)
		{
			if (!(e.e.mod & SDL_KMOD_CTRL))
			{
				m_cursor.position.column--;
			}
			else
			{
				auto& row = m_text[m_cursor.position.row];
				while (m_cursor.position.column > 0 && row[m_cursor.position.column - 1] == ' ')
				{
					m_cursor.position.column--;
				}
				while (m_cursor.position.column > 0 && row[m_cursor.position.column - 1] != ' ')
				{
					m_cursor.position.column--;
				}
			}
			m_cursor_dirty = true;
		}
		else if (m_cursor.position.row > 0)
		{
			m_cursor.position.row--;
			m_cursor.position.column = 0;
			if (m_cursor.position.row < m_text.size())
			{
				m_cursor.position.column = m_text[m_cursor.position.row].size();
			}
			m_cursor_dirty = true;
		}
		if (e.e.mod & SDL_KMOD_SHIFT)
		{
			m_selection_anchor = m_cursor;
			m_selection_active = m_selection_start != m_selection_anchor;
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
				m_selection_start = m_cursor;
			}
		}
		else
		{
			m_selection_active = false;
		}

		// handle motion
		if (m_cursor.position.row < m_text.size())
		{
			const auto& row = m_text[m_cursor.position.row];
			if (m_cursor.position.column < row.size())
			{
				if (!(e.e.mod & SDL_KMOD_CTRL))
				{
					m_cursor.position.column++;
				}
				else
				{
					while (m_cursor.position.column < row.size() && row[m_cursor.position.column] != ' ')
					{
						m_cursor.position.column++;
					}
					while (m_cursor.position.column < row.size() && row[m_cursor.position.column] == ' ')
					{
						m_cursor.position.column++;
					}
				}
			}
			else
			{
				m_cursor.position.row++;
				m_cursor.position.column = 0;
			}
			m_cursor_dirty = true;
		}
		else
		{
			m_cursor.position.row++;
			m_cursor_dirty = true;
		}
		if (e.e.mod & SDL_KMOD_SHIFT)
		{
			m_selection_anchor = m_cursor;
			m_selection_active = m_selection_start != m_selection_anchor;
		}
		return true;
	}
	else if (key == SDL_SCANCODE_UP)
	{
		// handle selection
		if (e.e.mod & SDL_KMOD_SHIFT)
		{
			if (!m_selection_active)
			{
				m_selection_active = true;
				m_selection_start = m_cursor;
			}
		}
		else
		{
			m_selection_active = false;
		}

		// handle motion
		if (m_cursor.position.row == 0)
		{
			m_cursor.position.column = 0;
			m_cursor_dirty = true;
		}
		else
		{
			m_cursor.position.row--;
			if (m_cursor.position.row < m_text.size())
			{
				const auto& row = m_text[m_cursor.position.row];
				if (m_cursor.position.column > row.size())
				{
					m_cursor.position.column = row.size();
				}
			}
			m_cursor_dirty = true;
		}
		if (e.e.mod & SDL_KMOD_SHIFT)
		{
			m_selection_anchor = m_cursor;
			m_selection_active = m_selection_start != m_selection_anchor;
		}
		return true;
	}
	else if (key == SDL_SCANCODE_DOWN)
	{
		// handle selection
		if (e.e.mod & SDL_KMOD_SHIFT)
		{
			if (!m_selection_active)
			{
				m_selection_active = true;
				m_selection_start = m_cursor;
			}
		}
		else
		{
			m_selection_active = false;
		}

		// handle motion
		m_cursor.position.row++;
		if (m_cursor.position.row >= m_text.size())
		{
			m_cursor.position.column = 0;
		}
		else
		{
			const auto& row = m_text[m_cursor.position.row];
			if (m_cursor.position.column > row.size())
			{
				m_cursor.position.column = row.size();
			}
			m_cursor_dirty = true;
		}
		m_cursor_dirty = true;
		if (e.e.mod & SDL_KMOD_SHIFT)
		{
			m_selection_anchor = m_cursor;
			m_selection_active = m_selection_start != m_selection_anchor;
		}
		return true;
	}
	else if (key == SDL_SCANCODE_RETURN)
	{
		m_logger.LogDebug("Return");
		DeleteSelection();
		if (m_cursor.position.row < m_text.size())
		{
			size_t return_pos = m_cursor.position.column;
			m_text.insert(m_text.begin() + m_cursor.position.row + 1,
				std::string{ m_text[m_cursor.position.row].begin() + return_pos, m_text[m_cursor.position.row].end() });
			m_text[m_cursor.position.row].resize(return_pos);
			m_cursor.position.column = 0;
			m_cursor.position.row++;
			m_cursor_dirty = true;
			UpdateScrollBar();
		}
		else
		{
			m_cursor.position.row++;
			m_cursor_dirty = true;
		}
		return true;
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
		InsertText(view);
		m_cursor_dirty = true;
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
		m_cursor_dirty = true;
		return true;
	}
	else if (key == SDL_SCANCODE_A && e.e.mod & SDL_KMOD_LCTRL)
	{
		if (!m_text.size())
		{
			return false;
		}
		m_selection_start.position = { 0,0 };

		size_t last_row = m_text.size() - 1;
		m_selection_anchor.position = { last_row, m_text[last_row].size() };
		m_cursor.position = m_selection_anchor.position;
		m_selection_active = true;
		m_cursor_dirty = true;
		UpdateOffset();
		return true;
	}
	else
	{
		m_logger.LogDebug("key {}", key);
	}
	return false;
}

void node::MultiLineEditControl::OnKeyboardFocusIn()
{
	m_focused = true;
	auto screen_position = GetGlobalPosition();
	auto rect = ToRect(GetSize().ToRectWithOrigin(screen_position));
	GetApp()->StartTextInput(rect);
}

void node::MultiLineEditControl::OnKeyboardFocusOut()
{
	m_focused = false;
	m_selection_active = false;
	GetApp()->StopTextInput();
}

void node::MultiLineEditControl::OnPositionRequested(float new_position)
{
	if (!m_text.size())
	{
		m_screen_top_row = 0;
	}
	else if (new_position < 0)
	{
		m_screen_top_row = 0;
	}
	else if (new_position > m_text.size() - 1)
	{
		m_screen_top_row = m_text.size() - 1;
	}
	else
	{
		m_screen_top_row = static_cast<size_t>(new_position);
	}
	UpdateScrollBar();
}

void node::MultiLineEditControl::OnSetSize(const WidgetSize& size)
{
	ScrollViewBase::OnSetSize(size);
	auto text_area = GetTextArea();
	for (auto& painter : m_painters)
	{
		painter.SetWidth(static_cast<size_t>(text_area.w));
	}
	UpdateScrollBar();
}

void node::MultiLineEditControl::UpdateScrollBar()
{
	if (!m_text.size())
	{
		SetScrollInfo(10, 0, 0);
		return;
	}
	int screen_height = static_cast<int>(GetTextArea().h / TTF_GetFontHeight(m_font));
	SetScrollInfo(static_cast<float>(screen_height), static_cast<float>(m_text.size() - 1), static_cast<float>(m_screen_top_row));
}

SDL_FRect node::MultiLineEditControl::GetTextArea() const
{
	SDL_FRect inner_rect{ GetSize().ToRect() };
	inner_rect.x += 1 + H_Margin;
	inner_rect.y += 1;
	inner_rect.w -= 2 + 2 * H_Margin + scrolltools::ScrollBar::scrollbar_width;
	inner_rect.h -= 2;
	return inner_rect;
}

void node::MultiLineEditControl::DrawText(SDL::Renderer& renderer)
{
	SDL_FRect inner_rect = GetTextArea();
	const SDL_Color text_color = renderer.GetColor(ColorRole::text_normal);
	size_t row = 0;
	int row_height = TTF_GetFontHeight(m_font);
	size_t rows_to_draw = static_cast<size_t>(inner_rect.h / row_height);
	while (m_painters.size() < rows_to_draw)
	{
		m_painters.push_back(TruncatedTextPainter{ m_font });
		m_painters.back().SetWidth(static_cast<size_t>(inner_rect.w));
		m_painters.back().SetPixelOffset(m_x_screen_offset);
	}
	while (m_painters.size() > rows_to_draw)
	{
		m_painters.pop_back();
	}
	
	auto selection_start = std::min(m_selection_start, m_selection_anchor);
	auto selection_end = std::max(m_selection_start, m_selection_anchor);
	
	float y = inner_rect.y;
	for (row = 0; row < rows_to_draw; row++)
	{
		size_t text_idx = row + m_screen_top_row;
		if (text_idx >= m_text.size())
		{
			break;
		}

		m_painters[row].SetText(m_text[text_idx]);
		if (m_selection_active && text_idx >= selection_start.position.row && text_idx <= selection_end.position.row)
		{
			DrawLineSelection(renderer, inner_rect, y, static_cast<float>(row_height), text_idx, selection_start, selection_end);
		}

		m_painters[row].Draw(renderer, { inner_rect.x, y }, text_color);
		y += static_cast<float>(row_height);
	}
}

void node::MultiLineEditControl::DrawCursor(SDL::Renderer& renderer)
{
	const SDL_FRect inner_rect = GetTextArea();
	int row_height = TTF_GetFontHeight(m_font);

	SDL_FPoint cursor_screen_origin{
		m_cursor.x - 2 + inner_rect.x - static_cast<float>(m_x_screen_offset),
		m_cursor.y + inner_rect.y - static_cast<float>(m_screen_top_row * row_height)
	};
	auto widget_rect = GetSize().ToRect();
	if (!SDL_PointInRectFloat(&cursor_screen_origin, &widget_rect))
	{
		return;
	}
	float cursor_height = static_cast<float>(TTF_GetFontHeight(m_font));
	SDL_FRect cursor_rect = SDL_FRect{ 
		cursor_screen_origin.x, cursor_screen_origin.y,
		2,cursor_height
	};

	const SDL_Color text_color = renderer.GetColor(ColorRole::text_normal);
	SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, text_color.a);
	SDL_RenderFillRect(renderer, &cursor_rect);
}

void node::MultiLineEditControl::DrawLineSelection(SDL::Renderer& renderer, 
	SDL_FRect text_rect, float y, float row_height, size_t text_index, const CursorData& selection_start, const CursorData& selection_end)
{
	float start_x = text_rect.x;
	float end_x = text_rect.w + text_rect.x;
	if (text_index == selection_start.position.row)
	{
		start_x = std::max(start_x, selection_start.x - m_x_screen_offset + text_rect.x);
	}
	if (text_index == selection_end.position.row)
	{
		end_x = std::min(end_x, selection_end.x - m_x_screen_offset + text_rect.x);
	}
	if (end_x <= start_x)
	{
		return; // zero area
	}
	const SDL_Color Blue = renderer.GetColor(ColorRole::blue_select);
	SDL_FRect selection_rect{ start_x, y, end_x - start_x, row_height };
	SDL_SetRenderDrawColor(renderer, Blue.r, Blue.g, Blue.b, Blue.a);
	SDL_RenderFillRect(renderer, &selection_rect);
}

void node::MultiLineEditControl::UpdateCursorsPixelPosition()
{
	m_cursor_dirty = false;
	UpdateCursorPixelPosition(m_cursor);
	UpdateOffset();

	if (m_selection_active)
	{
		UpdateCursorPixelPosition(m_selection_start);
		UpdateCursorPixelPosition(m_selection_anchor);
	}
}

void node::MultiLineEditControl::UpdateCursorPixelPosition(CursorData& cursor)
{
	cursor.y = static_cast<float>(TTF_GetFontHeight(m_font) * cursor.position.row);
	cursor.y = std::max<float>(cursor.y, 1);
	cursor.x = 1;
	if (cursor.position.column && cursor.position.row < m_text.size())
	{
		if (!m_text[cursor.position.row].size())
		{
			return;
		}
		assert(cursor.position.column <= m_text[cursor.position.row].size());

		int w{}, h{};
		if (TTF_GetStringSize(m_font, m_text[cursor.position.row].c_str(), cursor.position.column, &w, &h))
		{
			cursor.x = static_cast<float>(w + 1);
		}
	}
}

void node::MultiLineEditControl::UpdateOffset()
{
	size_t width = static_cast<size_t>(GetTextArea().w);
	if (m_cursor.position.column == 0)
	{
		SetOffset(0);
		return;
	}
	assert(m_cursor.position.row < m_text.size());
	if (m_cursor.x > min_cursor_distance_from_side && width > min_cursor_distance_from_side &&
		m_cursor.x > width + m_x_screen_offset - min_cursor_distance_from_side)
	{
		SetOffset(static_cast<size_t>(m_cursor.x) - width + min_cursor_distance_from_side);
	}
	else if (width > min_cursor_distance_from_side && m_x_screen_offset > m_cursor.x)
	{
		if (m_cursor.x > min_cursor_distance_from_side)
		{
			SetOffset(static_cast<size_t>(m_cursor.x) - min_cursor_distance_from_side);
		}
		else
		{
			SetOffset(static_cast<size_t>(0));
		}
	}
}

void node::MultiLineEditControl::SetOffset(size_t offset)
{
	m_x_screen_offset = offset;
	for (auto& painter : m_painters)
	{
		painter.SetPixelOffset(offset);
	}
}

bool node::MultiLineEditControl::DeleteSelection()
{
	if (!m_selection_active)
	{
		return false;
	}
	m_selection_active = false;
	if (!m_text.size())
	{
		return false;
	}
	auto selection_start_pos = std::min(m_selection_start, m_selection_anchor);
	auto selection_end_pos = std::max(m_selection_start, m_selection_anchor);
	if (selection_start_pos.position.row >= m_text.size())
	{
		// nothing selected, just move cursor to selection start
		m_cursor.position = selection_start_pos.position;
		return true;
	}
	if (selection_end_pos.position.row >= m_text.size())
	{
		selection_end_pos.position.row = m_text.size() - 1;
		selection_end_pos.position.column = m_text[m_text.size() - 1].size();
	}
	while (selection_end_pos.position.row - selection_start_pos.position.row > 1)
	{
		// remove lines between the two selectors, untill both are two concecutive lines
		m_text.erase(m_text.begin() + selection_end_pos.position.row - 1);
		selection_end_pos.position.row -= 1;
	}
	if (selection_end_pos.position.row - selection_start_pos.position.row == 1)
	{
		// on two concecutive lines, stitch them
		auto& first_row = m_text[selection_start_pos.position.row];
		first_row.erase(first_row.begin() + selection_start_pos.position.column, first_row.end());
		auto& second_row = m_text[selection_end_pos.position.row];
		first_row.insert(first_row.begin() + selection_start_pos.position.column,
			second_row.begin() + selection_end_pos.position.column, second_row.end());
		m_text.erase(m_text.begin() + selection_end_pos.position.row);
	}
	else // 0 difference
	{
		// erase what's in the middle
		auto& row = m_text[selection_start_pos.position.row];
		row.erase(row.begin() + selection_start_pos.position.column,
			row.begin() + selection_end_pos.position.column);
	}
	m_cursor.position = selection_start_pos.position;
	m_selection_start.position = selection_start_pos.position;
	m_selection_anchor.position = selection_start_pos.position;
	UpdateScrollBar();
	return true;
}

bool node::MultiLineEditControl::CopySelection()
{
	if (!m_selection_active)
	{
		return false;
	}
	if (!m_text.size())
	{
		return false;
	}
	auto* app = GetApp();
	if (!app)
	{
		return false;
	}

	std::string copy_string;
	auto selection_start_pos = std::min(m_selection_start, m_selection_anchor);
	auto selection_end_pos = std::max(m_selection_start, m_selection_anchor);
	while (selection_start_pos.position.row < selection_end_pos.position.row)
	{
		// copy lines till start and end on the same line
		if (selection_start_pos.position.row < m_text.size())
		{
			auto& row = m_text[selection_start_pos.position.row];
			copy_string.insert(copy_string.end(), 
				row.begin() + selection_start_pos.position.column, row.end());
		}
		copy_string.insert(copy_string.end(), '\n');
		selection_start_pos.position.row += 1;
		selection_start_pos.position.column = 0;
	}
	if (selection_start_pos.position.row < m_text.size())
	{
		auto& row = m_text[selection_start_pos.position.row];
		copy_string.insert(copy_string.end(), 
			row.begin() + selection_start_pos.position.column,
			row.begin() + selection_end_pos.position.column);
	}
	return app->SetClipboardText(copy_string);
}

bool node::MultiLineEditControl::InsertText(std::string_view view)
{
	if (!view.size())
	{
		return false;
	}
	if (m_text.size() <= m_cursor.position.row)
	{   
		// fill m_text to m_text.size() > m_cusor_position.row
		while (m_text.size() <= m_cursor.position.row)
		{
			m_text.push_back({});
		}
		UpdateScrollBar();
	}
	auto separator_position = view.find('\n');
	while (separator_position != view.npos)
	{
		// move text in line to next line
		{
			auto& row_text = m_text[m_cursor.position.row];
			assert(m_cursor.position.column <= row_text.size());
			if (m_cursor.position.column == 0)
			{
				m_text.insert(m_text.begin() + m_cursor.position.row, "");
			}
			else if (m_cursor.position.column == row_text.size())
			{
				m_text.insert(m_text.begin() + m_cursor.position.row + 1, "");
			}
			else
			{
				// move the rest of line to next row
				std::string next_line{ row_text.begin() + m_cursor.position.column, row_text.end() };
				row_text.erase(row_text.begin() + m_cursor.position.column, row_text.end());
				m_text.insert(m_text.begin() + m_cursor.position.row + 1, std::move(next_line));
			}
		}
		// add text up to the \n
		auto& row_text = m_text[m_cursor.position.row];
		std::string_view line;
		if (separator_position != 0)
		{
			line = view.substr(0, separator_position);
		}
		if (line.size() && line[line.size() - 1] == '\r')
		{
			// remove the \r
			line = line.substr(0, line.size() - 1);
		}
		row_text.insert(row_text.begin() + m_cursor.position.column, line.begin(), line.end());
		// move to next line
		m_cursor.position.row += 1;
		m_cursor.position.column = 0;
		view = view.substr(separator_position + 1);
		separator_position = view.find('\n');
 	}
	// add remaining text after all \n to the current line
	auto& row_text = m_text[m_cursor.position.row];
	row_text.insert(row_text.begin() + m_cursor.position.column, view.begin(), view.end());
	m_cursor.position.column = m_cursor.position.column + view.size();
	m_selection_start.position = m_cursor.position;
	m_selection_anchor.position = m_cursor.position;
	return true;
}

node::MultiLineEditControl::CharacterPosition node::MultiLineEditControl::GetCharacterAtScreenPosition(SDL_FPoint position)
{
	auto text_rect = GetTextArea();
	size_t clicked_row = 0;
	if (position.y > text_rect.y)
	{
		clicked_row = static_cast<size_t>((position.y - text_rect.y) / TTF_GetFontHeight(m_font)) + m_screen_top_row;
	}
	if (position.x <= text_rect.x || clicked_row >= m_text.size())
	{
		return CharacterPosition{clicked_row, 0, 0};
	}
	int click_pos_x = static_cast<int>(position.x - text_rect.x + m_x_screen_offset);
	int extent = 0;
	size_t count = 0;
	auto& text = m_text[clicked_row];
	if (!TTF_MeasureString(m_font, text.c_str(), text.size(), click_pos_x, &extent, &count))
	{
		// failed for some reason
		return CharacterPosition{ clicked_row, 0, 0 };
	}
	size_t new_cursor_pos = count;
	size_t pixel_offset = extent;
	if (count && static_cast<size_t>(count) < text.size())
	{
		// compare it with next character too
		int w = 0, h = 0;
		TTF_GetStringSize(m_font, text.c_str(), new_cursor_pos + 1, &w, &h);
		if (click_pos_x - extent > w - click_pos_x)
		{
			new_cursor_pos++;
			pixel_offset = w;
		}
	}
	return {clicked_row, new_cursor_pos, pixel_offset };
}
