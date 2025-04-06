#include "LogView.hpp"


class node::LogView::TextLine
{
public:
	TextLine(TTF_Font* font, float width) : m_font{ font }, m_width{ width } {}
	void SetText(std::string str);
	void SetWidth(float width);
	float GetHeight();
	void Draw(SDL::Renderer& renderer, const SDL_FPoint& point, SDL_Color color);

private:
	void AssurePainters();

	TTF_Font* m_font;
	float m_width;
	std::vector<TextPainter> m_painters;
	std::string m_text;
};

void node::LogView::TextLine::SetText(std::string str)
{
	if (str == m_text)
	{
		return;
	}
	m_text = std::move(str);
	m_painters.clear();
}

void node::LogView::TextLine::SetWidth(float width)
{
	if (m_width == width)
	{
		return;
	}
	m_width = width;
	m_painters.clear();
}

float node::LogView::TextLine::GetHeight()
{
	assert(m_font);
	AssurePainters();
	return static_cast<float>(TTF_GetFontHeight(m_font) * std::max(m_painters.size(), size_t{ 1 }));
}

void node::LogView::TextLine::Draw(SDL::Renderer& renderer, const SDL_FPoint& point, SDL_Color color)
{
	assert(m_font);
	AssurePainters();
	float y_offset = point.y;
	float line_height = static_cast<float>(TTF_GetFontHeight(m_font));
	for (auto& line : m_painters)
	{
		line.Draw(renderer, { point.x, y_offset }, color);
		y_offset += line_height;
	}
}

void node::LogView::TextLine::AssurePainters()
{

	if (m_painters.size() != 0)
	{
		return;
	}
	assert(m_font);

	std::string_view text_remaining = m_text;
	
	while (text_remaining.size())
	{
		int measured_width{};
		size_t measured_length{};
		[[maybe_unused]] bool success = TTF_MeasureString(m_font, text_remaining.data(), text_remaining.size(), static_cast<int>(m_width), &measured_width, &measured_length);
		assert(success);

		if (measured_length == text_remaining.size())
		{
			m_painters.emplace_back(m_font);
			m_painters.back().SetText(text_remaining);
			break;
		}

		size_t last_space = text_remaining.rfind(' ', measured_length);
		if (last_space == std::string_view::npos)
		{
			m_painters.emplace_back(m_font);
			m_painters.back().SetText(text_remaining);
			break;
		}
		m_painters.emplace_back(m_font);
		m_painters.back().SetText(text_remaining.substr(0, last_space));

		if (last_space + 1 >= text_remaining.size())
		{
			break;
		}
		text_remaining = text_remaining.substr(last_space + 1);
	}

}

node::LogView::LogView(const WidgetSize & size, TTF_Font* font, node::Widget * parent)
	:ScrollViewBase{size, parent}, m_font{font}
{
	SetScrollStrength(1);
}

node::LogView::~LogView() = default;

void node::LogView::OnSetSize(const WidgetSize& size)
{
	ScrollViewBase::OnSetSize(size);
	auto DrawArea = GetContainedAreaSize();
	for (auto& line : m_text_lines)
	{
		line.SetWidth(DrawArea.w - x_margin);
	}
	if (m_log_lines.size())
	{
		auto scroll_pos = m_first_line_idx == npos ? m_log_lines.size() - 1 : m_first_line_idx;
		SetScrollInfo(GetWidgetLinesHeight(), static_cast<float>(m_log_lines.size() - 1), static_cast<float>(scroll_pos));
	}
}

void node::LogView::OnPositionRequested(float new_position)
{
	if (m_log_lines.size() == 0)
	{
		return;
	}
	if (new_position < 0)
	{
		new_position = 0;
	}
	if (static_cast<float>(m_log_lines.size() - 1) < static_cast<size_t>(new_position))
	{
		m_first_line_idx = static_cast<size_t>(-1);
		SetScrollPosition(static_cast<float>(m_log_lines.size() - 1));
	}
	else
	{
		m_first_line_idx = static_cast<size_t>(new_position);
		SetScrollPosition(new_position);
	}
}

void node::LogView::AddLine(logging::LogLine line)
{
	m_log_lines.push_back(std::move(line));
	auto scroll_pos = m_first_line_idx == npos ? m_log_lines.size() - 1 : m_first_line_idx;
	SetScrollInfo(GetWidgetLinesHeight(), static_cast<float>(m_log_lines.size() - 1), static_cast<float>(scroll_pos));
}

void node::LogView::OnDraw(SDL::Renderer& renderer)
{
	DrawOutline(renderer);
	DrawLines(renderer);
}

void node::LogView::DrawOutline(SDL::Renderer& renderer)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	const auto& rect = GetSize().ToRect();
	SDL_RenderRect(renderer, &rect);
}

void node::LogView::DrawLines(SDL::Renderer& renderer)
{
	if (!m_log_lines.size())
	{
		return;
	}
	assert(m_font);
	if (m_first_line_idx == npos)
	{
		DrawLinesUpFrom(renderer, m_log_lines.size() - 1);
	}
	else
	{
		DrawLinesDownFrom(renderer, m_first_line_idx);
	}
}

void node::LogView::DrawLinesDownFrom(SDL::Renderer& renderer, size_t start_idx)
{
	float current_y = 0;
	size_t text_index{};
	SDL_Color Black{ 50,50,50,255 };
	SDL_Color Red{ 180, 50, 50, 255 };
	auto DrawArea = GetContainedAreaSize();
	for (text_index = 0; text_index + start_idx < m_log_lines.size() &&
		current_y < GetSize().h; text_index++)
	{
		if (text_index == m_text_lines.size())
		{
			m_text_lines.emplace_back(m_font, DrawArea.w - x_margin);
		}
		auto& painter = m_text_lines[text_index];
		painter.SetText(m_log_lines[text_index + start_idx].text);
		int priority = m_log_lines[text_index + start_idx].priority;
		painter.Draw(renderer, { x_margin, current_y }, priority >= SDL_LOG_PRIORITY_ERROR ? Red : Black);
		current_y += painter.GetHeight();
	}
	while (text_index > m_text_lines.size())
	{
		m_text_lines.pop_back();
	}
}

void node::LogView::DrawLinesUpFrom(SDL::Renderer& renderer, size_t end_idx)
{
	if (m_text_lines.size() == 0)
	{
		m_text_lines.emplace_back(m_font, GetSize().w - x_margin);
	}
	auto DrawArea = GetContainedAreaSize();

	float current_y = GetSize().h;
	size_t text_index = m_text_lines.size();;
	SDL_Color Black{ 50,50,50,255 };
	SDL_Color Red{ 180, 50, 50, 255 };
	for (size_t index_offset_up = 0; end_idx - index_offset_up < m_log_lines.size() &&
		current_y > 0; index_offset_up++)
	{
		if (text_index != 0)
		{
			text_index--;
		}
		else
		{
			m_text_lines.insert(m_text_lines.begin(), TextLine{ m_font, DrawArea.w - x_margin });
		}
		auto& painter = m_text_lines[text_index];
		painter.SetText(m_log_lines[end_idx - index_offset_up].text);
		int priority = m_log_lines[end_idx - index_offset_up].priority;
		current_y -= painter.GetHeight();
		painter.Draw(renderer, { x_margin, current_y }, priority >= SDL_LOG_PRIORITY_ERROR ? Red : Black);
	}
	while (text_index != 0)
	{
		m_text_lines.erase(m_text_lines.begin());
		text_index--;
	}
}

float node::LogView::GetWidgetLinesHeight() const
{
	return GetSize().h / TTF_GetFontHeight(m_font);
}