#include "DialogControls.hpp"
#include "toolgui/Scene.hpp"
#include "toolgui/Application.hpp"

std::vector<std::string> node::DialogLabel::SplitToLinesofWidth(const std::string& str, TTF_Font* font, int width)
{
	std::vector<std::string> lines;
	std::string_view sv{ str };
	while (sv.size())
	{
		size_t captured_letters = 0;
		int extent = 0;
		if (!TTF_MeasureString(font, sv.data(), sv.size(), width, &extent, &captured_letters))
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

node::DialogLabel::DialogLabel(std::vector<std::string> lines, const WidgetSize& size, TTF_Font* font, Dialog* parent)
	:DialogControl{ size, parent }, m_lines{ std::move(lines) }, m_font{ font }, m_color{ 50, 50, 50, 255 }
{
	assert(m_font);
	assert(parent);
}

void node::DialogLabel::OnDraw(SDL::Renderer& renderer)
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
	int font_height = TTF_GetFontHeight(m_font);
	assert(m_lines.size() == m_painters.size());
	for (size_t i = 0; i < m_lines.size(); i++)
	{
		const auto& line = m_lines[i];
		auto&& painter = m_painters[i];

		painter.SetText(line);

		SDL_FPoint text_start{ 0 , static_cast<float>(y) };
		painter.Draw(renderer, text_start, m_color);
		y += font_height + LinesMargin;
	}

}

void node::DialogLabel::SetText(std::vector<std::string> lines)
{
	m_lines = std::move(lines);
	m_painters.clear();
}

node::PropertyEditControl::PropertyEditControl(std::string name, int name_width, 
	std::string initial_value, const WidgetSize& size, TTF_Font* title_font, TTF_Font* error_font, Dialog* parent)
	:DialogControl{ size, parent },
	m_edit{ std::move(initial_value), {size.w - name_width, EditHeight}, title_font, this },
	m_name{ std::move(name) },
	m_painter{ title_font },
	m_error_painter{ error_font },
	m_name_width{ name_width }
{
	m_edit.SetPosition({ static_cast<float>(m_name_width), 0 });
	m_painter.SetText(m_name);
	SetFocusProxy(&m_edit);
	assert(parent);
}

void node::PropertyEditControl::SetErrorText(std::string line)
{
	m_error_painter.SetText(std::move(line));
}

void node::PropertyEditControl::OnDraw(SDL::Renderer& renderer)
{
	SDL_Color Black = { 50, 50, 50, 255 };
	SDL_FPoint text_start{};
	m_painter.Draw(renderer, text_start, Black);	
	text_start.y += m_edit.GetSize().h;
	m_error_painter.Draw(renderer, text_start, ErrorColor);
}

void node::PropertyEditControl::OnSetSize(const WidgetSize& size)
{
	DialogControl::OnSetSize(size);
	m_edit.SetSize({ GetSize().w - m_name_width, GetSize().h});
}

MI::ClickEvent node::PropertyEditControl::OnLMBDown(MouseButtonEvent& e)
{
	UNUSED_PARAM(e);
	return MI::ClickEvent::CLICKED;
}

void node::SeparatorControl::OnDraw(SDL::Renderer& renderer)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	auto rect = GetSize().ToRect();
	SDL_RenderFillRect(renderer, &rect);
}
