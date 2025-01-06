#pragma once

#include "toolgui/Widget.hpp"

namespace node
{

class StackedWidget : public Widget
{
public:
	StackedWidget(const SDL_FRect& rect, Widget* parent);
	void Draw(SDL_Renderer* renderer) override;

	int32_t AddWidget(std::unique_ptr<Widget> widget);
	bool DeleteWidget(int32_t index);

	bool SetCurrentIndex(Widget* ptr);
	bool SetCurrentIndex(int32_t index);
	Widget* GetCurrentWidget() const;
	int32_t GetCurrentIndex() const { return m_current_widget_index; }

	Widget* GetTabWidget(int32_t index);
	int32_t GetWidgetIndex(Widget* widget);

	int32_t WidgetsCount() const { return static_cast<int32_t>(m_widgets.size()); }

	static constexpr int32_t npos = -1;
protected:
	void OnSetRect(const SDL_FRect& rect) override;
	Widget* OnGetInteractableAtPoint(const SDL_FPoint& point) override;
private:
	std::vector<std::unique_ptr<Widget>> m_widgets;
	int32_t m_current_widget_index = npos;
};

}