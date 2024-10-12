#pragma once

#include "toolgui/Widget.hpp"
#include "SDL_Framework/SDLCPP.hpp"
#include <variant>
#include <functional>
#include "SDL_Framework/Utility.hpp"

namespace node
{

class ToolBarButton : public Widget
{
public:
	static constexpr int width = 40;
	static constexpr int height = 40;
	static constexpr int Hmargin = 10;

	ToolBarButton(const SDL_Rect& rect, Scene* parent, std::string name = {});
	~ToolBarButton() override;
	const std::string& GetName() const noexcept { return m_name; }
	void Draw(SDL_Renderer* renderer) override;
protected:
	virtual bool IsDisabled() { return false; }
	void OnMouseOut() override final;
	void OnMouseIn() override final;
	virtual void OnButonClicked();
	MI::ClickEvent OnLMBDown(const SDL_Point& current_mouse_point) override final;
	MI::ClickEvent OnLMBUp(const SDL_Point& current_mouse_point) override final;

	bool b_hovered = false;
	bool b_held_down = false;
	std::string m_name{};
	SDLTexture m_name_texture;
	std::unique_ptr<RoundRectPainter> m_painter_outer;
	std::unique_ptr<RoundRectPainter> m_painter_inner;
};

class ToolBar: public Widget
{
public:
	static constexpr int height = 50;

	ToolBar(const SDL_Rect& rect, Scene* parent);
	~ToolBar() override;
	void AddButton(std::unique_ptr<ToolBarButton> button, int position = -1);
	void AddSeparator(int position = -1);
	node::ToolBarButton* GetButton(const std::string& name);
	void Draw(SDL_Renderer* renderer) override;


	struct ToolBarSeparator {
		static constexpr int width = 2;
		static constexpr int VMargin = 5;
		int position_x = 0;
	};
	using ToolBarElement = typename std::variant<std::unique_ptr<ToolBarButton>, ToolBarSeparator>;
protected:
	Widget* OnGetInteractableAtPoint(const SDL_Point& point) override;
	void OnSetRect(const SDL_Rect& rect) override;
private:
	void RepositionButtons();
	std::vector<ToolBarElement> m_buttons;
};


class ToolBarCommandButton : public ToolBarButton
{
public:
	ToolBarCommandButton(const SDL_Rect& rect, Scene* parent,
		std::string name = {}, std::function<void()> func = {}, std::function<bool()> Active = []()->bool {return true; });
	void OnButonClicked() override;
protected:
	bool IsDisabled() override { return !m_isActive(); }
private:
	std::function<void()> m_action;
	std::function<bool()> m_isActive;
};

}