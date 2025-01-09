#pragma once

#include "toolgui/Widget.hpp"
#include "SDL_Framework/SDLCPP.hpp"
#include <variant>
#include <functional>
#include "SDL_Framework/Utility.hpp"
#include "SDL_Framework/SVGRasterizer.hpp"
#include "toolgui/ToolTipWidgetMixin.hpp"

namespace node
{

class ToolBar;

class ToolBarButton : public Widget, public mixin::TooltipMixin<ToolBarButton>
{
public:
	static constexpr float width = 32;
	static constexpr float height = 32;
	static constexpr float Hmargin = 8;

	ToolBarButton(const WidgetSize& size, ToolBar* parent, std::string name = {});
	~ToolBarButton() override;
	const std::string& GetName() const noexcept { return m_name; }
	void OnDraw(SDL::Renderer& renderer) override;
	void SetSVGPath(std::string path);
	void SetDescription(std::string description);
	ToolBar* GetToolBar()
	{
		return m_parent_toolbar;
	}
	void SetToolBar(ToolBar* toolbar);
protected:
	virtual bool IsDisabled() { return false; }
	void OnMouseOut(MouseHoverEvent& e) override final;
	void OnMouseIn(MouseHoverEvent& e) override final;
	void OnMouseMove(MouseHoverEvent& e) override;
	virtual void OnButonClicked();
	MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override final;
	MI::ClickEvent OnLMBUp(MouseButtonEvent& e) override final;
private:
	bool b_hovered = false;
	bool b_held_down = false;
	std::string m_name{};
	std::optional<TextPainter> m_text_painter;
	std::optional<SVGRasterizer> m_svg_painter;
	std::unique_ptr<RoundRectPainter> m_painter_outer;
	std::unique_ptr<RoundRectPainter> m_painter_inner;
	ToolBar* m_parent_toolbar;
};

class ToolBar: public Widget
{
public:
	static constexpr int height = 40;

	ToolBar(const WidgetSize& size, Widget* parent);
	~ToolBar() override;
	void AddButton(std::unique_ptr<ToolBarButton> button, int position = -1);
	void AddSeparator(int position = -1);
	node::ToolBarButton* GetButton(const std::string& name);
	void OnDraw(SDL::Renderer& renderer) override;


	struct ToolBarSeparator {
		static constexpr float width{ 2.0f };
		static constexpr float VMargin{ 5.0f };
		float position_x = 0;
	};
	using ToolBarElement = typename std::variant<std::unique_ptr<ToolBarButton>, ToolBarSeparator>;
protected:

private:
	void RepositionButtons();
	std::vector<ToolBarElement> m_buttons;
};


class ToolBarCommandButton : public ToolBarButton
{
public:
	ToolBarCommandButton(const WidgetSize& size, ToolBar* parent,
		std::string name = {}, std::function<void()> func = {}, std::function<bool()> Active = []()->bool {return true; });
	void OnButonClicked() override;
protected:
	bool IsDisabled() override { return !m_isActive(); }
private:
	std::function<void()> m_action;
	std::function<bool()> m_isActive;
};

}