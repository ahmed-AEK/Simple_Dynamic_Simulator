#include "ToolBar.hpp"
#include "Scene.hpp"
#include "Application.hpp"
#include "SDL_Framework/Utility.hpp"
#include "ToolTipWidget.hpp"

node::ToolBar::ToolBar(const SDL_Rect& rect, Scene* parent)
	:Widget(rect, parent)
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

void node::ToolBar::Draw(SDL_Renderer * renderer)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &GetRect());
	SDL_Rect inner_rect = GetRect();
	inner_rect.x += 2;
	inner_rect.y += 2;
	inner_rect.w -= 4;
	inner_rect.h -= 4;
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(renderer, &inner_rect);
	for (auto&& button : m_buttons)
	{
		if (isButton(button))
		{
			AsButton(button)->Draw(renderer);
		}
		else
		{
			SDL_Rect separator_rect{ AsSeparator(button).position_x, inner_rect.y + ToolBarSeparator::VMargin, 
				ToolBarSeparator::width, inner_rect.h - 2 * ToolBarSeparator::VMargin };
			SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
			SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
			SDL_RenderFillRect(renderer, &separator_rect);
		}
	}
}

node::Widget* node::ToolBar::OnGetInteractableAtPoint(const SDL_Point& point)
{
	for (auto&& button : m_buttons)
	{
		if (!isButton(button))
		{
			continue;
		}
		if (auto result = AsButton(button)->GetInteractableAtPoint(point))
		{
			return result;
		}
	}
	return Widget::OnGetInteractableAtPoint(point);
}

void node::ToolBar::OnSetRect(const SDL_Rect& rect)
{
	Widget::OnSetRect(rect);
	RepositionButtons();
	
}

void node::ToolBar::RepositionButtons()
{
	int position_x = ToolBarButton::Hmargin + GetRect().x;
	int position_y = GetRect().y;
	for (auto&& button : m_buttons)
	{
		if (isButton(button))
		{
			AsButton(button)->SetRect({ position_x, position_y + 4, ToolBarButton::width, ToolBarButton::height });
			position_x += ToolBarButton::Hmargin + ToolBarButton::width;
		}
		else
		{
			AsSeparator(button).position_x = position_x;
			position_x += ToolBarSeparator::width + ToolBarButton::Hmargin;
		}

	}
}


node::ToolBarButton::ToolBarButton(const SDL_Rect& rect, Scene* parent, std::string name)
	:Widget(rect, parent), m_name{std::move(name)}, 
	m_painter_outer{std::make_unique<RoundRectPainter>()},
	m_painter_inner{ std::make_unique<RoundRectPainter>() }
{
}

node::ToolBarButton::~ToolBarButton()
{
	HideToolTip();
}

void node::ToolBarButton::Draw(SDL_Renderer* renderer)
{
	const int thickness = 2;
	{
		SDL_Color color_outer = b_hovered ? SDL_Color{ 255,150,0,255 } : SDL_Color{ 180, 180, 180, 255 };
		const int radius = 10;
		SDL_Color inactive_color = b_held_down ? SDL_Color{ 230, 230, 230, 255 } : SDL_Color{ 255, 255, 255, 255 };
		SDL_Color color_inner = IsDisabled() ? SDL_Color{230, 230, 230, 255} : inactive_color;
		ThickFilledRoundRect(renderer, GetRect(), radius, thickness, color_outer, color_inner, 
			*m_painter_outer, *m_painter_inner);
	}


	SDL_Rect inner_rect = GetRect();
	inner_rect.x += thickness;
	inner_rect.y += thickness;
	inner_rect.w -= 2 * thickness;
	inner_rect.h -= 2 * thickness;
	auto text_drawer = [&]() 
		{
			if (!m_text_painter)
			{
				m_text_painter = TextPainter{ GetScene()->GetApp()->getFont().get() };
				if (m_name.size())
				{
					m_text_painter->SetText(std::string{ m_name[0] });
				}
			}

			SDL_Color Black = { 50, 50, 50, 255 };
			SDL_Rect text_rect = m_text_painter->GetRect(renderer, Black);
			text_rect.x = inner_rect.x + inner_rect.w / 2 - text_rect.w / 2;
			text_rect.y = inner_rect.y + inner_rect.h / 2 - text_rect.h / 2;
			m_text_painter->Draw(renderer, SDL_Point{ text_rect.x, text_rect.y }, Black);
		};
	if (!m_svg_painter)
	{
		text_drawer();
	}
	else
	{
		// draw the svg
		m_svg_painter->SetSize(inner_rect.w - 8, inner_rect.h - 8);
		if (!m_svg_painter->Draw(renderer, inner_rect.x + 4, inner_rect.y + 4))
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
	m_description = description;
}

void node::ToolBarButton::OnMouseOut()
{
	b_hovered = false;
	b_held_down = false;
	HideToolTip();
	if (m_updateTaskId)
	{
		GetScene()->GetApp()->RemoveUpdateTask(m_updateTaskId);
		m_updateTaskId = 0;
	}
}

void node::ToolBarButton::OnMouseIn()
{
	b_hovered = true;
	m_last_action_time = SDL_GetTicks64();
	if (!m_updateTaskId && m_description.size())
	{
		m_updateTaskId = GetScene()->GetApp()->AddUpdateTask(UpdateTask::FromWidget(*this, [this]() {this->InternalUpdateToolTip(); }));
	}
}

void node::ToolBarButton::OnMouseMove(const SDL_Point& current_point)
{
	m_last_mouse_pos = current_point;
}

void node::ToolBarButton::OnButonClicked()
{
}

MI::ClickEvent node::ToolBarButton::OnLMBDown(const SDL_Point& current_mouse_point)
{
	UNUSED_PARAM(current_mouse_point);
	b_held_down = true;
	return MI::ClickEvent::CLICKED;
}

MI::ClickEvent node::ToolBarButton::OnLMBUp(const SDL_Point& current_mouse_point)
{
	UNUSED_PARAM(current_mouse_point);
	if (b_held_down)
	{
		OnButonClicked();
	}
	b_held_down = false;
	return MI::ClickEvent::CLICKED;
}


void node::ToolBarButton::InternalUpdateToolTip()
{
	uint64_t current_time = SDL_GetTicks64();
	if (current_time - m_last_action_time > 500 && m_updateTaskId)
	{
		auto toolTipWidget = std::make_unique<ToolTipWidget>(GetScene()->GetApp()->getFont().get(), m_description, SDL_Rect{ m_last_mouse_pos.x, m_last_mouse_pos.y, 1,1 }, GetScene());
		m_toolTipWidget = toolTipWidget->GetMIHandlePtr();
		GetScene()->ShowToolTip(std::move(toolTipWidget));
		GetScene()->GetApp()->RemoveUpdateTask(m_updateTaskId);
		m_updateTaskId = 0;
	}
}

void node::ToolBarButton::HideToolTip()
{
	auto* scene = GetScene();
	if (scene && m_toolTipWidget)
	{
		scene->HideToolTip(m_toolTipWidget.GetObjectPtr());
		m_toolTipWidget = nullptr;
	}
}

node::ToolBarCommandButton::ToolBarCommandButton(const SDL_Rect& rect, Scene* parent, 
	std::string name, std::function<void()> func, std::function<bool()> Active)
	:ToolBarButton{rect, parent, name}, m_action{std::move(func)}, m_isActive{Active}
{
}

void node::ToolBarCommandButton::OnButonClicked()
{
	if (m_isActive())
	{
		m_action();
	}
}