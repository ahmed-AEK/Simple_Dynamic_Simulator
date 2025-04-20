#pragma once

#include "toolgui/ScrollView.hpp"
#include "SDL_Framework/Utility.hpp"
#include "PluginAPI/Logger.hpp"

namespace node
{
class MultiLineEditControl : public ScrollViewBase
{
public:
	MultiLineEditControl(const WidgetSize& size, TTF_Font* font, Widget* parent);
	const std::vector<std::string>& GetText() const { return m_text; }
	void SetText(std::vector<std::string> text);
	~MultiLineEditControl() override;
protected:
	void OnDraw(SDL::Renderer& renderer) override;

	MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;
	MI::ClickEvent OnLMBUp(MouseButtonEvent& e) override;
	void OnMouseMove(MouseHoverEvent& e) override;

	bool OnChar(TextInputEvent& e) override;
	bool OnKeyPress(KeyboardEvent& e) override;
	virtual void OnKeyboardFocusIn() override;
	virtual void OnKeyboardFocusOut() override;
	void OnPositionRequested(float new_position) override;
	void OnSetSize(const WidgetSize& size) override;

private:
	void UpdateScrollBar();
	SDL_FRect GetTextArea() const;
	void DrawText(SDL::Renderer& renderer);
	void DrawCursor(SDL::Renderer& renderer);


	struct CursorPosition
	{
		size_t row;
		size_t column;
		constexpr bool operator==(const CursorPosition&) const = default;
		constexpr auto operator<=>(const CursorPosition&) const = default;
	};
	struct CursorData
	{
		CursorPosition position;
		float x;
		float y;
		constexpr bool operator==(const CursorData& rhs) const
		{
			return this->position == rhs.position;
		}
		constexpr auto operator<=>(const CursorData& rhs) const
		{
			return this->position <=> rhs.position;
		}
	};
	void DrawLineSelection(SDL::Renderer& renderer, SDL_FRect text_rect, 
		float y, float row_height, size_t text_index,
		const CursorData& selection_start, const CursorData& selection_end);
	void UpdateCursorsPixelPosition();
	void UpdateCursorPixelPosition(CursorData& cursor);
	void UpdateOffset();
	void SetOffset(size_t offset);
	bool DeleteSelection();
	bool CopySelection();
	bool InsertText(std::string_view view);

	struct CharacterPosition
	{
		size_t character_row;
		size_t character_column;
		size_t pixel_offset;
	};
	CharacterPosition GetCharacterAtScreenPosition(SDL_FPoint position);

	RoundRectPainter m_outer_painter;
	RoundRectPainter m_inner_painter;
	std::vector<std::string> m_text;
	std::vector<TruncatedTextPainter> m_painters;
	TTF_Font* m_font = nullptr;

	size_t m_screen_top_row = 0;
	size_t m_x_screen_offset = 0;

	CursorData m_cursor{};
	CursorData m_selection_start{};
	CursorData m_selection_anchor{};

	bool m_focused = false;
	bool m_cursor_dirty = true;
	bool m_dragging = false;
	bool m_selection_active = false;
	logging::Logger m_logger = logger(logging::LogCategory::GUI);

	static constexpr int H_Margin = 3;
	static constexpr size_t min_cursor_distance_from_side = 15;
};
}
