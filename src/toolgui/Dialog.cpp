#include "Dialog.hpp"
#include "SDL_Framework/SDLCPP.hpp"
#include "Scene.hpp"
#include "Application.hpp"
#include "ToolBar.hpp"

node::Dialog::Dialog(std::string title, const SDL_Rect& rect, Scene* parent)
	:Widget{ rect, parent }, m_title_painter{parent->GetApp()->getFont().get()}, m_title{ std::move(title) }
{
	assert(parent);
	m_title_painter.SetText(m_title);
}

node::Dialog::~Dialog()
{
}

void node::Dialog::Draw(SDL_Renderer* renderer)
{
	
	DrawOutline(renderer, GetRect());

	SDL_Rect banner_rect = GetTitleBarRect();
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &banner_rect);
	banner_rect.x += 1;
	banner_rect.y += 1;
	banner_rect.w -= 2;
	banner_rect.h -= 2;
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(renderer, &banner_rect);

	DrawTitle(renderer, SDL_Point{ banner_rect.x + 5, banner_rect.y });
	DrawXButton(renderer, GetXButtonRect());

	if (m_toolbar)
	{
		m_toolbar->Draw(renderer);
	}
	for (auto&& button : m_buttons)
	{
		button->Draw(renderer);
	}
	for (auto&& control : m_controls)
	{
		control->Draw(renderer);
	}
}

void node::Dialog::AddControl(std::unique_ptr<DialogControl> control, int position)
{
	if (position == -1 || static_cast<size_t>(position) >= m_controls.size())
	{
		m_controls.push_back(std::move(control));
		if (m_controls.back()->GetSizingMode() == DialogControl::SizingMode::expanding)
		{
			m_var_height_elements++;
		}
	}
	else
	{
		auto it = m_controls.insert(m_controls.begin() + position, std::move(control));
		if (it->get()->GetSizingMode() == DialogControl::SizingMode::expanding)
		{
			m_var_height_elements++;
		}
	}
	
	ResizeToFitChildren();

}

void node::Dialog::AddButton(std::string title, std::function<void()> callback)
{
	m_buttons.push_back(std::make_unique<DialogButton>(std::move(title), std::move(callback), SDL_Rect{ 0,0,80,ButtonHeight }, GetScene()));
	ResizeToFitChildren();
}

void node::Dialog::OnClose()
{
	GetScene()->PopDialog(this);
}

void node::Dialog::OnOk()
{
	GetScene()->PopDialog(this);
}

void node::Dialog::OnMouseMove(const SDL_Point& current_mouse_point)
{
	const auto& X_rect = GetXButtonRect();
	if (SDL_PointInRect(&current_mouse_point, &X_rect))
	{
		b_mouse_on_close = true;
	}
	else
	{
		b_mouse_on_close = false;
		b_being_closed = false;
	}

	if (std::holds_alternative<TitleDrag>(m_dragData))
	{
		const auto& drag_data = std::get<TitleDrag>(m_dragData);
		int x_distance = current_mouse_point.x - drag_data.drag_mouse_start_position.x;
		int y_distance = current_mouse_point.y - drag_data.drag_mouse_start_position.y;
		SDL_Rect new_rect{ drag_data.drag_edge_start_position.x + x_distance, drag_data.drag_edge_start_position.y + y_distance , GetRect().w, GetRect().h };
		SDL_Rect title_rect = GetTitleBarRect();
		if (new_rect.x + new_rect.w > GetScene()->GetRect().w)
		{
			new_rect.x = GetScene()->GetRect().w - new_rect.w;
		}
		if (new_rect.x < 0)
		{
			new_rect.x = 0;
		}
		if (new_rect.y + title_rect.h > GetScene()->GetRect().h)
		{
			new_rect.y = GetScene()->GetRect().h - title_rect.h;
		}
		if (new_rect.y < 0)
		{
			new_rect.y = 0;
		}

		SetRect(new_rect);
	}
	if (std::holds_alternative<ResizeDrag>(m_dragData))
	{
		const auto& drag_data = std::get<ResizeDrag>(m_dragData);
		SDL_Rect old_Rect = GetRect();
		SDL_Rect new_Rect = old_Rect;
		switch (drag_data.mode)
		{
		case ResizeDrag::DragMode::grip:
		{
			new_Rect.w = std::max(current_mouse_point.x - old_Rect.x, drag_data.min_size.x);
			new_Rect.h = std::max(current_mouse_point.y - old_Rect.y, drag_data.min_size.y);
			break;
		}
		case ResizeDrag::DragMode::top:
		{
			new_Rect.h = std::max(drag_data.drag_edge_start_position.y - current_mouse_point.y, drag_data.min_size.y);
			new_Rect.y = drag_data.drag_edge_start_position.y - new_Rect.h;
			break;
		}
		}

		if (new_Rect.w != old_Rect.w || new_Rect.h != old_Rect.h)
		{
			SetRect(new_Rect);
		}
	}
}

MI::ClickEvent node::Dialog::OnLMBDown(const SDL_Point& current_mouse_point)
{
	if (GetScene())
	{
		GetScene()->BumpDialogToTop(this);
	}

	const auto& X_btn_rect = GetXButtonRect();
	if (SDL_PointInRect(&current_mouse_point, &X_btn_rect))
	{
		b_being_closed = true;
		return MI::ClickEvent::CLICKED;
	}

	auto rect = GetRect();
	if (m_resizable)
	{
		if (current_mouse_point.y >= rect.y && current_mouse_point.y <= (rect.y + top_resize_grip_height))
		{
			m_dragData = ResizeDrag{
				SDL_Point{rect.x + rect.w, rect.y + rect.h},
				CalculateMinSize(),
				ResizeDrag::DragMode::top
			};
			return MI::ClickEvent::CAPTURE_START;
		}
	}

	const auto& banner_rect = GetTitleBarRect();
	if (SDL_PointInRect(&current_mouse_point, &banner_rect))
	{
		m_dragData = TitleDrag{
			 SDL_Point{ GetRect().x, GetRect().y },
			 current_mouse_point
		};
		return MI::ClickEvent::CAPTURE_START;
	}
	
	if (m_resizable)
	{
		SDL_Rect ResizeGripRect = GetResizeGripRect();
		if (SDL_PointInRect(&current_mouse_point, &ResizeGripRect))
		{
			m_dragData = ResizeDrag{
				SDL_Point{rect.x + rect.w, rect.y + rect.h},
				CalculateMinSize(),
				ResizeDrag::DragMode::grip
			};
			return MI::ClickEvent::CAPTURE_START;
		}
	}

	return MI::ClickEvent::NONE;
}

MI::ClickEvent node::Dialog::OnLMBUp(const SDL_Point& current_mouse_point)
{
	UNUSED_PARAM(current_mouse_point);
	if (b_being_closed)
	{
		const auto& X_rect = GetXButtonRect();
		if (SDL_PointInRect(&current_mouse_point, &X_rect))
		{
			TriggerClose();
			return MI::ClickEvent::CLICKED;
		}
	}
	if (BeingDragged())
	{
		m_dragData = std::monostate{};
		return MI::ClickEvent::CAPTURE_END;
	}
	return MI::ClickEvent::NONE;
}

void node::Dialog::OnMouseOut()
{
	b_being_closed = false;
	b_mouse_on_close = false;
}

node::Widget* node::Dialog::OnGetInteractableAtPoint(const SDL_Point& point)
{
	if (m_toolbar)
	{
		if (auto ptr = m_toolbar->GetInteractableAtPoint(point))
		{
			return ptr;
		}
	}
	if (m_resizable)
	{
		SDL_Rect ResizeGripRect = GetResizeGripRect();
		if (SDL_PointInRect(&point, &ResizeGripRect))
		{
			return this;
		}
	}
	for (auto&& control : m_controls)
	{
		if (auto ptr = control->GetInteractableAtPoint(point))
		{
			return ptr;
		}
	}
	for (auto&& button : m_buttons)
	{
		if (auto object = button->GetInteractableAtPoint(point))
		{
			return object;
		}
	}
	return this;
}

void node::Dialog::OnSetRect(const SDL_Rect& rect)
{
	Widget::OnSetRect(rect);
	SDL_Point min_size = CalculateMinSize();
	m_excess_height = GetRect().h - min_size.y;
	RepositionControls();
	RepositionButtons();
}

void node::Dialog::RepositionControls()
{
	auto title_bar_rect = GetTitleBarRect();
	int x = ControlsMargin + GetRect().x;
	int y = GetRect().y + title_bar_rect.h + ControlsMargin;
	if (m_toolbar)
	{
		SDL_Rect self_rect = GetRect();
		m_toolbar->SetRect({ self_rect.x, self_rect.y + title_bar_rect.h, self_rect.w, ToolBar::height });
		y += ToolBar::height;
	}

	for (auto&& control : m_controls)
	{
		const SDL_Rect& old_rect = control->GetRect();
		if (control->GetSizingMode() == DialogControl::SizingMode::expanding)
		{
			control->SetRect({ x, y, GetRect().w, control->GetSizeHint().h + static_cast<int>(m_excess_height / m_var_height_elements) });
		}
		else
		{
			control->SetRect({ x, y, old_rect.w, old_rect.h });
		}
		y += control->GetRect().h + ControlsMargin;
	}
	
}

void node::Dialog::RepositionButtons()
{
	const int buttons_widths = [&]()
	{
		int buttons_widths = 0;
		for (const auto& button : m_buttons)
		{
			buttons_widths += button->GetRect().w;
		}
		return buttons_widths;
	}();
	int current_x = GetRect().x + GetRect().w - buttons_widths - ButtonsMargin * static_cast<int>(m_buttons.size());
	const int current_y = GetRect().y + GetRect().h - ButtonsMargin - ButtonHeight;
	for (auto&& button : m_buttons)
	{
		const auto& current_rect = button->GetRect();
		button->SetRect({ current_x, current_y, current_rect.w, current_rect.h});
		current_x += current_rect.w + ButtonsMargin;
	}
}

void node::Dialog::ResizeToFitChildren()
{
	SDL_Point min_size = CalculateMinSize();
	int current_width = GetRect().w;
	int current_height = GetRect().h;
	m_excess_height = std::max(min_size.y, current_height) - min_size.y;
	if (current_width < min_size.x || current_height < min_size.y)
	{
		SetRect({ GetRect().x, GetRect().y, std::max(min_size.x, current_width), std::max(min_size.y, current_height) });
	}
	else
	{
		RepositionControls();
		RepositionButtons();
	}
}

void node::Dialog::DrawTitle(SDL_Renderer* renderer, const SDL_Point& start )
{
	SDL_Color Black = { 50, 50, 50, 255 };
	SDL_Rect text_rect = m_title_painter.GetRect(renderer, Black);
	SDL_Point text_start{ start.x, start.y + 40 / 2 - text_rect.h / 2 };
	m_title_painter.Draw(renderer, text_start, Black);
}

void node::Dialog::DrawXButton(SDL_Renderer* renderer, const SDL_Rect& rect)
{
	SDL_Rect base = rect;
	SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
	SDL_RenderFillRect(renderer, &base);
	base.x += 1;
	base.y += 1;
	base.w -= 2;
	base.h -= 2;
	if (b_being_closed)
	{
		SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
	}
	else if (b_mouse_on_close && !BeingDragged())
	{
		SDL_SetRenderDrawColor(renderer, 255, 60, 60, 255);
	}
	else
	{
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	}
	SDL_RenderFillRect(renderer, &base);

	const int dist_from_side = 8;
	if (b_mouse_on_close && !BeingDragged())
	{
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	}
	else
	{
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	}
	SDL_RenderDrawLine(renderer, base.x + dist_from_side, base.y + dist_from_side, base.x + base.w - dist_from_side, base.y + base.h - dist_from_side);
	SDL_RenderDrawLine(renderer, 1 + base.x + dist_from_side, base.y + dist_from_side, 1 + base.x + base.w - dist_from_side, base.y + base.h - dist_from_side);
	SDL_RenderDrawLine(renderer, base.x + dist_from_side, base.y + base.h - dist_from_side, base.x + base.w - dist_from_side, base.y + dist_from_side);
	SDL_RenderDrawLine(renderer, 1 + base.x + dist_from_side, base.y + base.h - dist_from_side, 1 + base.x + base.w - dist_from_side, base.y + dist_from_side);

}

void node::Dialog::DrawOutline(SDL_Renderer* renderer, const SDL_Rect& rect)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &rect);
	SDL_Rect inner_rect = rect;
	inner_rect.x += 1;
	inner_rect.y += 1;
	inner_rect.w -= 2;
	inner_rect.h -= 2;
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(renderer, &inner_rect);

	if (m_resizable)
	{
		// draw resize handle in bottom right corner
		std::array<SDL_Vertex, 3> arrow_verts =
			std::array<SDL_Vertex, 3>{
			SDL_Vertex{ static_cast<float>(inner_rect.x + inner_rect.w - 20), static_cast<float>(inner_rect.y + inner_rect.h)
			, {0,0,0,0}, {0,0} },
				SDL_Vertex{ static_cast<float>(inner_rect.x + inner_rect.w), static_cast<float>(inner_rect.y + inner_rect.h),
				{0,0,0,0}, {0,0} },
				SDL_Vertex{ static_cast<float>(inner_rect.x + inner_rect.w), static_cast<float>(inner_rect.y + inner_rect.h - 20)
				, {0,0,0,0}, {0,0} },
		};
		SDL_RenderGeometry(renderer, nullptr, arrow_verts.data(), 3, nullptr, 0);
	}
}

SDL_Rect node::Dialog::GetTitleBarRect() const
{
	const auto& this_rect = GetRect();
	return SDL_Rect{ this_rect.x, this_rect.y, this_rect.w, 40 };
}

void node::Dialog::SetToolbar(std::unique_ptr<ToolBar> toolbar)
{
	m_toolbar = std::move(toolbar);
	SDL_Rect self_rect = GetRect();
	auto title_bar_rect = GetTitleBarRect();
	m_toolbar->SetRect({ self_rect.x, self_rect.y + title_bar_rect.h, self_rect.w, ToolBar::height });
}

SDL_Rect node::Dialog::GetXButtonRect() const
{
	const auto& this_rect = GetRect();
	return SDL_Rect{ this_rect.x + this_rect.w - 40, this_rect.y + 5, 30, 30 };
}

SDL_Rect node::Dialog::GetResizeGripRect() const
{
	auto&& rect = GetRect();
	return { rect.x + rect.w - 20, rect.y + rect.h - 20, 20, 20 };
}

bool node::Dialog::BeingDragged() const
{
	return !std::holds_alternative<std::monostate>(m_dragData);
}

SDL_Point node::Dialog::CalculateMinSize() const
{
	int min_width = 0;
	int min_height = ControlsMargin + GetTitleBarRect().h;
	if (m_toolbar)
	{
		min_height += ToolBar::height;
	}
	for (const auto& control : m_controls)
	{
		SDL_Rect size_hint = control->GetSizeHint();
		min_height += size_hint.h + ControlsMargin;
		min_width = std::max(min_width, size_hint.w);
	}
	if (m_buttons.size())
	{
		min_height += ButtonHeight + ButtonsMargin;
	}
	min_width += ControlsMargin * 2; // pad both sides
	int buttons_width = ButtonsMargin; // pad left
	for (const auto& button : m_buttons)
	{
		buttons_width += button->GetRect().w + ButtonsMargin;
	}
	min_width = std::max(min_width, buttons_width);
	min_width = std::max(min_width, MinWidth);
	return { min_width, min_height };
}

node::DialogButton::DialogButton(std::string text, std::function<void()> OnClick, const SDL_Rect& rect, Scene* scene)
	:Widget{rect, scene}, m_text{text}, m_onClick{OnClick}
{
}

void node::DialogButton::Draw(SDL_Renderer* renderer)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &GetRect());
	SDL_Rect inner_rect{ GetRect() };
	inner_rect.x += 1;
	inner_rect.y += 1;
	inner_rect.w -= 2;
	inner_rect.h -= 2;
	if (b_being_clicked)
	{
		SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
	}
	else
	{
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	}
	SDL_RenderFillRect(renderer, &inner_rect);

	SDL_Color Black = { 50, 50, 50, 255 };
	auto textSurface = SDLSurface{ TTF_RenderText_Blended(GetScene()->GetApp()->getFont().get(), m_text.c_str(), Black) };
	auto textTexture = SDLTexture{ SDL_CreateTextureFromSurface(renderer, textSurface.get()) };

	SDL_Rect text_rect{};
	SDL_QueryTexture(textTexture.get(), NULL, NULL, &text_rect.w, &text_rect.h);
	text_rect.x = GetRect().x + (GetRect().w - text_rect.w) / 2;
	text_rect.y = GetRect().y + (GetRect().h - text_rect.h) / 2;
	SDL_RenderCopy(renderer, textTexture.get(), NULL, &text_rect);
}

void node::DialogButton::OnMouseOut()
{
	b_being_clicked = false;
}

MI::ClickEvent node::DialogButton::OnLMBDown(const SDL_Point& current_mouse_point)
{
	UNUSED_PARAM(current_mouse_point);
	b_being_clicked = true;
	return MI::ClickEvent::CLICKED;
}

MI::ClickEvent node::DialogButton::OnLMBUp(const SDL_Point& current_mouse_point)
{
	UNUSED_PARAM(current_mouse_point);
	if (b_being_clicked)
	{
		m_onClick();
		return MI::ClickEvent::CLICKED;
	}
	return MI::ClickEvent::NONE;
}

node::DialogControl::DialogControl(const SDL_Rect& rect, Scene* parent)
	:Widget{ rect, parent }, m_size_hint{ rect }
{
}
