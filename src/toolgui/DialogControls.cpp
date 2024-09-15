#include "DialogControls.hpp"
#include "toolgui/Scene.hpp"
#include "toolgui/Application.hpp"

std::vector<std::string> node::DialogLabel::SplitToLinesofWidth(const std::string& str, TTF_Font* font, int width)
{
	std::vector<std::string> lines;
	std::string_view sv{ str };
	while (sv.size())
	{
		int captured_letters = 0;
		int extent = 0;
		int result = TTF_MeasureUTF8(font, sv.data(), width, &extent, &captured_letters);
		if (result)
		{
			SDL_Log("Failed to measure font extent");
			return lines;
		}
		if (static_cast<size_t>(captured_letters) == sv.size())
		{
			lines.push_back(std::string{ sv });
			return lines;
		}
		else
		{
			auto pos = sv.rfind(' ', captured_letters);
			if (pos == sv.npos)
			{
				lines.push_back(std::string{ sv.substr(0,captured_letters) });
				sv = sv.substr(captured_letters);
			}
			else
			{
				lines.push_back(std::string{ sv.substr(0,pos) });
				sv = sv.substr(pos + 1);
			}

		}
	}
	return lines;
}

node::DialogLabel::DialogLabel(std::vector<std::string> lines, const SDL_Rect& rect, TTF_Font* font, Scene* parent)
	:DialogControl{ rect, parent }, m_lines{ std::move(lines) }, m_font{ font }
{
	assert(m_font);
	assert(parent);
}

void node::DialogLabel::Draw(SDL_Renderer* renderer)
{
	int y = 0;

	if (!m_painters.size())
	{
		m_painters.reserve(m_lines.size());
		for (const auto& line : m_lines)
		{
			UNUSED_PARAM(line);
			m_painters.emplace_back(m_font);
		}
	}
	int font_height = TTF_FontHeight(m_font);
	assert(m_lines.size() == m_painters.size());
	for (size_t i = 0; i < m_lines.size(); i++)
	{
		const auto& line = m_lines[i];
		auto&& painter = m_painters[i];

		SDL_Color Black = { 50, 50, 50, 255 };
		painter.SetText(line);

		SDL_Point text_start{ GetRect().x, GetRect().y + y };
		painter.Draw(renderer, text_start, Black);
		y += font_height + LinesMargin;
	}

}

node::PropertyEditControl::PropertyEditControl(std::string name, int name_width, std::string initial_value, const SDL_Rect& rect, Scene* parent)
	:DialogControl{ rect, parent },
	m_edit{ std::move(initial_value), {rect.x + m_name_width, rect.y, rect.w - m_name_width, rect.h}, parent },
	m_name{ std::move(name) },
	m_painter{ GetScene()->GetApp()->getFont().get() },
	m_name_width{ name_width }
{
	m_painter.SetText(m_name);
	assert(parent);
}

void node::PropertyEditControl::Draw(SDL_Renderer* renderer)
{
	{
		SDL_Color Black = { 50, 50, 50, 255 };
		SDL_Point text_start{ GetRect().x, GetRect().y };
		m_painter.Draw(renderer, text_start, Black);
	}
	m_edit.Draw(renderer);
}

void node::PropertyEditControl::OnSetRect(const SDL_Rect& rect)
{
	DialogControl::OnSetRect(rect);
	m_edit.SetRect({ rect.x + m_name_width, rect.y, rect.w - m_name_width, rect.h });
}

node::Widget* node::PropertyEditControl::OnGetInteractableAtPoint(const SDL_Point& point)
{
	if (auto ptr = m_edit.GetInteractableAtPoint(point))
	{
		return ptr;
	}
	return this;
}

void node::SeparatorControl::Draw(SDL_Renderer* renderer)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &GetRect());
}
