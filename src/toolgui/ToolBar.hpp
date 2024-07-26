#pragma once

#include "toolgui/Widget.hpp"

namespace node
{

class ToolBarButton : public Widget
{
public:
	static constexpr int width = 40;
	static constexpr int height = 40;
	static constexpr int Hmargin = 10;

	ToolBarButton(const SDL_Rect& rect, Scene* parent);
	~ToolBarButton();
	std::string_view GetName() noexcept;
	void Draw(SDL_Renderer* renderer) override;
protected:
	void OnMouseOut() override;
	void OnMouseIn() override;
	bool b_hovered = false;
	std::string m_name{};
};

class ToolBar: public Widget
{
public:
	static constexpr int height = 50;

	ToolBar(const SDL_Rect& rect, Scene* parent);
	~ToolBar();
	void AddButton(std::unique_ptr<ToolBarButton> button, int position = -1);
	
	void Draw(SDL_Renderer* renderer) override;
protected:
	Widget* OnGetInteractableAtPoint(const SDL_Point& point) override;
	void OnSetRect(const SDL_Rect& rect) override;
private:
	std::vector<std::unique_ptr<ToolBarButton>> m_buttons;
};

}