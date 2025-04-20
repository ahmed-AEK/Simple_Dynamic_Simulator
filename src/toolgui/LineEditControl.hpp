#pragma once

#include "toolgui/Widget.hpp"
#include "SDL_Framework/Utility.hpp"
#include "PluginAPI/Logger.hpp"

namespace node
{

class LineEditControl : public Widget
{
public:
	LineEditControl(std::string initial_value, const WidgetSize& size, TTF_Font* font, Widget* parent);
	const std::string& GetValue() const { return m_value; }
	~LineEditControl() override;
protected:
	void OnDraw(SDL::Renderer& renderer) override;
	MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;
	MI::ClickEvent OnLMBUp(MouseButtonEvent& e) override;
	void OnMouseMove(MouseHoverEvent& e) override;

	bool OnChar(TextInputEvent& e) override;
	bool OnKeyPress(KeyboardEvent& e) override;
	virtual void OnKeyboardFocusIn() override;
	virtual void OnKeyboardFocusOut() override;
	void OnSetSize(const WidgetSize& size) override;
private:
	void DrawSelectionRect(SDL::Renderer& renderer);
	void ReCalculateCursorsPixelPosition();
	void ReCalculateMainCursorPixelPosition();
	void ReCalculateSelectionCursorsPixelPosition(const size_t position, size_t& pixel_position);
	void UpdateOffset();
	struct CharacterPosition
	{
		size_t character_offset;
		size_t pixel_offset;
	};
	CharacterPosition GetCharacterAtScreenPosition(float x);
	SDL_FRect GetTextArea() const;
	bool DeleteSelection();
	bool CopySelection();
	RoundRectPainter m_outer_painter;
	RoundRectPainter m_inner_painter;
	std::string m_value;
	TruncatedTextPainter m_painter;
	size_t m_cursor_position;
	size_t m_cursor_pixel_position{ 1 };
	bool m_focused = false;
	bool m_dragging = false;
	bool m_selection_active = false;
	logging::Logger m_logger = logger(logging::LogCategory::GUI);
	size_t m_selection_start = 0;
	size_t m_selection_anchor = 0;
	size_t m_selection_start_pixel_pos = 0;
	size_t m_selection_anchor_pixel_pos = 0;
	static constexpr int H_Margin = 5;
	static constexpr size_t min_cursor_distance_from_side = 15;
};
}
