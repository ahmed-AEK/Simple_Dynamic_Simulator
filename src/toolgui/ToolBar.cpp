#include "ToolBar.hpp"
#include "Scene.hpp"
#include "Application.hpp"
#include "SDL_Framework/Utility.hpp"
#include "SDL_Framework/SDL_Math.hpp"
#include "toolgui/ToolTipWidgetMixin_impl.hpp"

#include <algorithm>

template class node::mixin::TooltipMixin<node::ToolBarButton>;

node::ToolBar::ToolBar(const WidgetSize& size, Widget* parent)
	:Widget(size, parent)
{
}

static bool isButton(const node::ToolBar::ToolBarElement& element)
{
	return std::holds_alternative<std::unique_ptr<node::ToolBarButton>>(element);
}

static std::unique_ptr<node::ToolBarButton>& AsButton(node::ToolBar::ToolBarElement& element)
{
	return std::get<std::unique_ptr<node::ToolBarButton>>(element);
}

static node::ToolBar::ToolBarSeparator& AsSeparator(node::ToolBar::ToolBarElement& element)
{
	return std::get<node::ToolBar::ToolBarSeparator>(element);
}

node::ToolBar::~ToolBar()
{
}

void node::ToolBar::AddButton(std::unique_ptr<ToolBarButton> button, int position)
{
	auto* ptr = button.get();
	if (position == -1)
	{
		m_buttons.push_back(std::move(button));
	}
	else if (static_cast<size_t>(position) < m_buttons.size())
	{
		m_buttons.insert(m_buttons.begin() + position, std::move(button));
	}
	else
	{
		m_buttons.push_back(std::move(button));
	}
	ptr->SetParent(this);
	ptr->SetFocusProxy(this);
	RepositionButtons();
}

void node::ToolBar::AddSeparator(int position)
{
	if (position == -1)
	{
		m_buttons.push_back(ToolBarSeparator{});
		return;
	}

	if (static_cast<size_t>(position) < m_buttons.size())
	{
		m_buttons.insert(m_buttons.begin() + position, std::move(ToolBarSeparator{}));
	}
	else
	{
		m_buttons.push_back(ToolBarSeparator{});
	}
}

node::ToolBarButton* node::ToolBar::GetButton(const std::string& name)
{
	auto it = std::find_if(m_buttons.begin(), m_buttons.end(), [&](auto&& button) { 
		if (isButton(button))
		{
			return AsButton(button)->GetName() == name;
		}
		return false;
		 });
	if (it != m_buttons.end())
	{
		return  AsButton(*it).get();
	}
	return nullptr;
}

void node::ToolBar::OnDraw(SDL::Renderer& renderer)
{
	SDL_FRect inner_rect = GetSize().ToRect();
	inner_rect.x += 2;
	inner_rect.y += 2;
	inner_rect.w -= 4;
	inner_rect.h -= 4;
	for (auto&& button : m_buttons)
	{
		if (isButton(button))
		{
			// button draws itself
		}
		else
		{
			SDL_FRect separator_rect{ AsSeparator(button).position_x, inner_rect.y + ToolBarSeparator::VMargin, 
				ToolBarSeparator::width, inner_rect.h - 2 * ToolBarSeparator::VMargin };
			SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
			SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
			SDL_RenderFillRect(renderer, &separator_rect);
		}
	}
}

void node::ToolBar::RepositionButtons()
{
	float position_x = ToolBarButton::Hmargin;
	float position_y = 0;
	for (auto&& button : m_buttons)
	{
		if (isButton(button))
		{
			AsButton(button)->SetPosition({ position_x, position_y + 4 });
			position_x += ToolBarButton::Hmargin + ToolBarButton::width;
		}
		else
		{
			AsSeparator(button).position_x = position_x;
			position_x += ToolBarSeparator::width + ToolBarButton::Hmargin;
		}
	}
}


node::ToolBarButton::ToolBarButton(const WidgetSize& size, ToolBar* parent, std::string name)
	:Widget(size, parent), m_name{std::move(name)}, 
	m_painter_outer{std::make_unique<RoundRectPainter>()},
	m_painter_inner{ std::make_unique<RoundRectPainter>() },
	m_parent_toolbar{parent}
{
}

node::ToolBarButton::~ToolBarButton()
{
}

void node::ToolBarButton::OnDraw(SDL::Renderer& renderer)
{
	const int thickness = 2;
	{
		const int radius = 8;
		SDL_Color outline_color{190, 190, 190, 255};
		SDL_Color inactive_color = b_held_down ? SDL_Color{ 210, 210, 210, 255 } : (b_hovered ? SDL_Color{ 230, 230, 230, 235 } : SDL_Color{ 255, 255, 255, 255 });
		SDL_Color color_inner = IsDisabled() ? SDL_Color{190, 190, 190, 255} : inactive_color;
		ThickFilledRoundRect(renderer, GetSize().ToRect(), radius, 2, outline_color, color_inner, *m_painter_outer, *m_painter_inner);
	}
	

	SDL_FRect inner_rect = GetSize().ToRect();
	inner_rect.x += thickness;
	inner_rect.y += thickness;
	inner_rect.w -= 2 * thickness;
	inner_rect.h -= 2 * thickness;
	auto text_drawer = [&]() 
		{
			if (!m_text_painter)
			{
				m_text_painter = TextPainter{ GetApp()->getFont().get() };
				if (m_name.size())
				{
					m_text_painter->SetText(std::string{ m_name[0] });
				}
			}

			SDL_Color Black = { 50, 50, 50, 255 };
			SDL_FRect text_rect = m_text_painter->GetRect(renderer, Black);
			text_rect.x = inner_rect.x + inner_rect.w / 2 - text_rect.w / 2;
			text_rect.y = inner_rect.y + inner_rect.h / 2 - text_rect.h / 2;
			m_text_painter->Draw(renderer, SDL_FPoint{ text_rect.x, text_rect.y }, Black);
		};
	if (!m_svg_painter)
	{
		text_drawer();
	}
	else
	{
		constexpr float margin = 3;
		// draw the svg
		m_svg_painter->SetSize(static_cast<int>(inner_rect.w - 2 * margin), static_cast<int>(inner_rect.h - 2 * margin));
		if (!m_svg_painter->Draw(renderer, inner_rect.x + margin, inner_rect.y + margin))
		{
			m_svg_painter = std::nullopt;
			text_drawer();
		}
	}
}

void node::ToolBarButton::SetSVGPath(std::string path)
{
	m_svg_painter = SVGRasterizer{ path , 0, 0};
	m_text_painter = std::nullopt;
}

void node::ToolBarButton::SetDescription(std::string description)
{
	SetToolTipDescription(std::move(description));
}

void node::ToolBarButton::SetToolBar(ToolBar* toolbar)
{
	m_parent_toolbar = toolbar;
	SetParent(toolbar);
}

void node::ToolBarButton::OnMouseOut(MouseHoverEvent& e)
{
	b_hovered = false;
	b_held_down = false;
	ToolTipMouseOut(e);
}

void node::ToolBarButton::OnMouseIn(MouseHoverEvent& e)
{
	b_hovered = true;
	ToolTipMouseIn(e);
}

void node::ToolBarButton::OnMouseMove(MouseHoverEvent& e)
{
	ToolTipMouseMove(e);
}

void node::ToolBarButton::OnButonClicked()
{
}

MI::ClickEvent node::ToolBarButton::OnLMBDown(MouseButtonEvent& e)
{
	UNUSED_PARAM(e);
	b_held_down = true;
	return MI::ClickEvent::CLICKED;
}

MI::ClickEvent node::ToolBarButton::OnLMBUp(MouseButtonEvent& e)
{
	UNUSED_PARAM(e);
	if (b_held_down)
	{
		OnButonClicked();
	}
	b_held_down = false;
	return MI::ClickEvent::CLICKED;
}

node::ToolBarCommandButton::ToolBarCommandButton(const WidgetSize& size, ToolBar* parent, 
	std::string name, std::function<void()> func, std::function<bool()> Active)
	:ToolBarButton{size, parent, name}, m_action{std::move(func)}, m_isActive{Active}
{
}

void node::ToolBarCommandButton::OnButonClicked()
{
	if (m_isActive())
	{
		m_action();
	}
}
