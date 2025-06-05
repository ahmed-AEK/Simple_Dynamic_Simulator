#include "Dialog.hpp"
#include "SDL_Framework/SDLCPP.hpp"
#include "SDL_Framework/SDL_Math.hpp"
#include "Scene.hpp"
#include "Application.hpp"
#include "ToolBar.hpp"

node::Dialog::Dialog(std::string title, const WidgetSize& size, Scene* parent)
	:Widget{ size, parent }, m_title_painter{parent->GetApp()->getFont().get()}, 
	m_X_painter{ parent->GetApp()->getFont().get() }, m_title{ std::move(title) },
	m_scene{parent}
{
	assert(parent);
	m_title_painter.SetText(m_title);
	m_X_painter.SetText("X");
	m_title_bar_painter.SetDrawSides(true, true, false, false);
}

node::Dialog::~Dialog()
{
}

void node::Dialog::OnDraw(SDL::Renderer& renderer)
{
	DrawOutline(renderer, GetSize().ToRect());

	SDL_FRect banner_rect = GetTitleBarRect();
	banner_rect.x += 1;
	banner_rect.y += 1;
	banner_rect.w -= 2;
	banner_rect.h -= 2;
	m_title_bar_painter.Draw(renderer, banner_rect, 8, renderer.GetColor(ColorRole::dialog_title_background));
	const SDL_Color separator_color = renderer.GetColor(ColorRole::frame_outline);
	SDL_SetRenderDrawColor(renderer, separator_color.r, separator_color.g, separator_color.b, 255);
	SDL_FRect title_separator_rect{ banner_rect.x, banner_rect.y + banner_rect.h, banner_rect.w, 1 };
	SDL_RenderFillRect(renderer, &title_separator_rect);

	DrawTitle(renderer, SDL_FPoint{ banner_rect.x + ControlsMargin, banner_rect.y });
	DrawXButton(renderer, GetXButtonRect());
}

void node::Dialog::AddControl(std::unique_ptr<DialogControl> control, int position)
{
	if (position == -1 || static_cast<size_t>(position) >= m_controls.size())
	{
		m_controls.push_back(std::move(control));
		m_controls.back()->SetParent(this);
		if (m_controls.back()->GetSizingMode() == DialogControl::SizingMode::expanding)
		{
			m_var_height_elements++;
		}
	}
	else
	{
		auto it = m_controls.insert(m_controls.begin() + position, std::move(control));
		it->get()->SetParent(this);
		if (it->get()->GetSizingMode() == DialogControl::SizingMode::expanding)
		{
			m_var_height_elements++;
		}
	}
	
	ResizeToFitChildren();

}

void node::Dialog::AddButton(std::string title, std::function<void()> callback)
{
	m_buttons.push_back(std::make_unique<DialogButton>(std::move(title), 
		m_title_painter.GetFont(), std::move(callback), WidgetSize{80,ButtonHeight}, this));
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

bool node::Dialog::OnKeyPress(KeyboardEvent& e)
{
	if (e.e.type != SDL_EVENT_KEY_DOWN)
	{
		return false;
	}
	switch (e.e.scancode)
	{
	case SDL_SCANCODE_ESCAPE:
	{
		TriggerClose();
		return true;
	}
	case SDL_SCANCODE_RETURN:
	case SDL_SCANCODE_KP_ENTER:
	{
		TriggerOk();
		return true;
	}
	default: 
	{
		break;
	}
	}
	return false;
}

node::Widget* node::Dialog::GetFocusable()
{
	for (auto&& control : m_controls)
	{
		if (auto focusable = control->GetFocusable())
		{
			return focusable;
		}
	}
	return this;
}

void node::Dialog::OnMouseMove(MouseHoverEvent& e)
{
	{
		const auto& X_rect = GetXButtonRect();
		SDL_FPoint current_mouse_point{ e.point() };
		if (SDL_PointInRectFloat(&current_mouse_point, &X_rect))
		{
			b_mouse_on_close = true;
		}
		else
		{
			b_mouse_on_close = false;
			b_being_closed = false;
		}

	}

	SDL_FPoint mouse_parent_point{ e.point() + GetPosition() };
	if (std::holds_alternative<TitleDrag>(m_dragData))
	{
		const auto& drag_data = std::get<TitleDrag>(m_dragData);
		float x_distance = mouse_parent_point.x - drag_data.drag_mouse_start_position.x;
		float y_distance = mouse_parent_point.y - drag_data.drag_mouse_start_position.y;
		SDL_FPoint new_pos{ drag_data.drag_edge_start_position.x + x_distance, drag_data.drag_edge_start_position.y + y_distance };
		SDL_FRect title_rect = GetTitleBarRect();
		SDL_FRect parent_Rect = GetParent()->GetSize().ToRect();
		if (new_pos.x + GetSize().w > parent_Rect.w)
		{
			new_pos.x = parent_Rect.w - GetSize().w;
		}
		if (new_pos.x < 0)
		{
			new_pos.x = 0;
		}
		if (new_pos.y + title_rect.h > parent_Rect.h)
		{
			new_pos.y = parent_Rect.h - title_rect.h;
		}
		if (new_pos.y < 0)
		{
			new_pos.y = 0;
		}

		SetPosition(new_pos);
	}
	if (std::holds_alternative<ResizeDrag>(m_dragData))
	{
		const auto& drag_data = std::get<ResizeDrag>(m_dragData);
		WidgetSize old_size = GetSize();
		auto new_size = old_size;
		SDL_FPoint old_position = GetPosition();
		auto new_position = old_position;

		switch (drag_data.mode)
		{
		case ResizeDrag::DragMode::grip:
		{
			new_size.w = std::max(mouse_parent_point.x - old_position.x, drag_data.min_size.w);
			new_size.h = std::max(mouse_parent_point.y - old_position.y, drag_data.min_size.h);
			if (new_size.w != old_size.w || new_size.h != old_size.h)
			{
				SetSize(new_size);
			}
			break;
		}
		case ResizeDrag::DragMode::top:
		{
			new_size.h = std::max(drag_data.drag_edge_start_position.y - mouse_parent_point.y, drag_data.min_size.h);
			new_position.y = drag_data.drag_edge_start_position.y - new_size.h;
			if (new_position.y != old_position.y)
			{
				SetPosition(new_position);
				if (new_size.w != old_size.w || new_size.h != old_size.h)
				{
					SetSize(new_size);
				}
			}
			break;
		}
		}
	}
}

MI::ClickEvent node::Dialog::OnLMBDown(MouseButtonEvent& e)
{
	if (GetScene())
	{
		GetScene()->BumpDialogToTop(this);
	}

	{
		SDL_FPoint current_mouse_point{ e.point() };
		const auto& X_btn_rect = GetXButtonRect();
		if (SDL_PointInRectFloat(&current_mouse_point, &X_btn_rect))
		{
			b_being_closed = true;
			return MI::ClickEvent::CLICKED;
		}
	}

	if (m_resizable)
	{
		// check top resize
		SDL_FPoint current_mouse_point{ e.point() };
		if (current_mouse_point.y >= 0 && current_mouse_point.y <= top_resize_grip_height)
		{
			m_dragData = ResizeDrag{
				GetSize().ToFPoint() + GetPosition(),
				CalculateMinSize(),
				ResizeDrag::DragMode::top
			};
			return MI::ClickEvent::CAPTURE_START;
		}
	}

	{
		// check title drag
		SDL_FPoint current_mouse_point{ e.point() };
		const auto& banner_rect = GetTitleBarRect();
		if (SDL_PointInRectFloat(&current_mouse_point, &banner_rect))
		{
			m_dragData = TitleDrag{
				 GetPosition(),
				 current_mouse_point + GetPosition()
			};
			return MI::ClickEvent::CAPTURE_START;
		}
	}
	
	if (m_resizable)
	{
		// check grip resize
		SDL_FRect ResizeGripRect = GetResizeGripRect();
		SDL_FPoint current_mouse_point{ e.point() };
		if (SDL_PointInRectFloat(&current_mouse_point, &ResizeGripRect))
		{
			m_dragData = ResizeDrag{
				GetSize().ToFPoint() + GetPosition(),
				CalculateMinSize(),
				ResizeDrag::DragMode::grip
			};
			return MI::ClickEvent::CAPTURE_START;
		}
	}

	return MI::ClickEvent::NONE;
}

MI::ClickEvent node::Dialog::OnLMBUp(MouseButtonEvent& e)
{
	SDL_FPoint current_mouse_point{ e.point() };
	if (b_being_closed)
	{
		const auto& X_rect = GetXButtonRect();
		if (SDL_PointInRectFloat(&current_mouse_point, &X_rect))
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

void node::Dialog::OnMouseOut(MouseHoverEvent&)
{
	b_being_closed = false;
	b_mouse_on_close = false;
}

node::Widget* node::Dialog::OnGetInteractableAtPoint(const SDL_FPoint& point)
{
	if (m_toolbar)
	{
		if (auto ptr = m_toolbar->GetInteractableAtPoint(point - m_toolbar->GetPosition()))
		{
			return ptr;
		}
	}
	if (m_resizable)
	{
		SDL_FRect ResizeGripRect = GetResizeGripRect();
		if (SDL_PointInRectFloat(&point, &ResizeGripRect))
		{
			return this;
		}
	}
	for (auto&& control : m_controls)
	{
		if (auto ptr = control->GetInteractableAtPoint(point - control->GetPosition()))
		{
			return ptr;
		}
	}
	for (auto&& button : m_buttons)
	{
		if (auto object = button->GetInteractableAtPoint(point - button->GetPosition()))
		{
			return object;
		}
	}
	return this;
}

void node::Dialog::OnSetSize(const WidgetSize& size)
{
	Widget::OnSetSize(size);
	const auto min_size = CalculateMinSize();
	m_excess_height = GetSize().h - min_size.h;
	RepositionControls();
	RepositionButtons();
}

void node::Dialog::RepositionControls()
{
	const auto title_bar_rect = GetTitleBarRect();
	float x = ControlsMargin;
	float y = title_bar_rect.h + ControlsMargin;
	if (m_toolbar)
	{
		m_toolbar->SetPosition({ 0, title_bar_rect.h });
		m_toolbar->SetSize({ GetSize().w, ToolBar::height});
		y += ToolBar::height;
	}

	for (auto&& control : m_controls)
	{
		if (control->GetSizingMode() == DialogControl::SizingMode::expanding)
		{
			control->SetPosition({ x, y });
			control->SetSize({ GetSize().w - ControlsMargin * 2, control->GetSizeHint().h + static_cast<int>(m_excess_height / m_var_height_elements) });
		}
		else
		{
			control->SetPosition({ x,y });
		}
		y += control->GetSize().h + ControlsMargin;
	}
	
}

void node::Dialog::RepositionButtons()
{
	const float buttons_widths = [&]()
	{
		float buttons_widths = 0;
		for (const auto& button : m_buttons)
		{
			buttons_widths += button->GetSize().w;
		}
		return buttons_widths;
	}();
	float current_x = GetSize().w - buttons_widths - ButtonsMargin * static_cast<int>(m_buttons.size());
	if (m_resizable)
	{
		current_x -= GetResizeGripRect().w;
	}
	const float current_y = GetSize().h - ButtonsMargin - ButtonHeight;
	for (auto&& button : m_buttons)
	{
		button->SetPosition({ current_x, current_y });
		current_x += button->GetSize().w + ButtonsMargin;
	}
}

void node::Dialog::ResizeToFitChildren()
{
	const WidgetSize min_size = CalculateMinSize();
	float current_width = GetSize().w;
	float current_height = GetSize().h;
	m_excess_height = std::max(min_size.h, current_height) - min_size.h;
	if (current_width < min_size.w || current_height < min_size.h)
	{
		SetSize({ std::max(min_size.w, current_width), std::max(min_size.h, current_height) });
	}
	else
	{
		RepositionControls();
		RepositionButtons();
	}
}

void node::Dialog::DrawTitle(SDL::Renderer& renderer, const SDL_FPoint& start )
{
	const SDL_Color text_color = renderer.GetColor(ColorRole::text_normal);
	SDL_FRect text_rect = m_title_painter.GetRect(renderer);
	SDL_FPoint text_start{ start.x, start.y + 30 / 2 - text_rect.h / 2 };
	m_title_painter.Draw(renderer, text_start, text_color);
}

void node::Dialog::DrawXButton(SDL::Renderer& renderer, const SDL_FRect& rect)
{
	SDL_FRect base = rect;
	const SDL_Color btn_outer_color = renderer.GetColor(ColorRole::btn_outline);
	SDL_Color btn_inner_color = btn_outer_color;

	if (b_being_closed)
	{
		btn_inner_color = renderer.GetColor(ColorRole::red_close_clicked);
	}
	else if (b_mouse_on_close && !BeingDragged())
	{
		btn_inner_color = renderer.GetColor(ColorRole::red_close);
	}
	else
	{
		btn_inner_color = renderer.GetColor(ColorRole::btn_normal);
	}
	ThickFilledRoundRect(renderer, base, 8, 1, btn_outer_color, btn_inner_color, m_X_btn_outer_painter, m_X_btn_inner_painter);

	SDL_Color x_color = renderer.GetColor(ColorRole::text_normal);

	base.x += 1;
	base.y += 1;
	base.w -= 2;
	base.h -= 2;
	SDL_FRect X_rect = m_X_painter.GetRect(renderer);
	X_rect.x = base.x + base.w / 2 - X_rect.w / 2;
	X_rect.y = base.y + base.h / 2 - X_rect.h / 2;
	m_X_painter.Draw(renderer, { X_rect.x, X_rect.y }, x_color);
}

void node::Dialog::DrawOutline(SDL::Renderer& renderer, const SDL_FRect& rect)
{
	SDL_FRect inner_rect = rect;
	const SDL_Color outline_color = renderer.GetColor(ColorRole::dialog_outline);
	const SDL_Color bg_color = renderer.GetColor(ColorRole::dialog_background);
	ThickFilledRoundRect(renderer, inner_rect, 8, 1, 
		outline_color, bg_color, m_outer_painter, m_inner_painter);
	inner_rect.x += 1;
	inner_rect.y += 1;
	inner_rect.w -= 2;
	inner_rect.h -= 2;


	if (m_resizable)
	{
		// draw resize handle in bottom right corner
		const auto handle_color = ToFColor(outline_color);
		std::array<SDL_Vertex, 3> arrow_verts =
			std::array<SDL_Vertex, 3>{
			SDL_Vertex{ {static_cast<float>(inner_rect.x + inner_rect.w - 20), static_cast<float>(inner_rect.y + inner_rect.h)},
				handle_color, {0,0}},
				SDL_Vertex{ {static_cast<float>(inner_rect.x + inner_rect.w), static_cast<float>(inner_rect.y + inner_rect.h)},
				handle_color, {0,0} },
				SDL_Vertex{ {static_cast<float>(inner_rect.x + inner_rect.w), static_cast<float>(inner_rect.y + inner_rect.h - 20)},
				handle_color, {0,0} },
		};
		SDL_RenderGeometry(renderer, nullptr, arrow_verts.data(), 3, nullptr, 0);
	}
}

SDL_FRect node::Dialog::GetTitleBarRect() const
{
	return SDL_FRect{ 0, 0, GetSize().w, 30};
}

void node::Dialog::SetToolbar(std::unique_ptr<ToolBar> toolbar)
{
	m_toolbar = std::move(toolbar);
	m_toolbar->SetParent(this);
	auto title_bar_rect = GetTitleBarRect();
	m_toolbar->SetPosition({ title_bar_rect.x, title_bar_rect.y + title_bar_rect.h });
	m_toolbar->SetSize({ GetSize().w, ToolBar::height});
}

void node::Dialog::SetResizeable(bool value)
{
	m_resizable = value;
	if (m_resizable)
	{
		m_outer_painter.SetDrawSides(true, true, false, true);
		m_inner_painter.SetDrawSides(true, true, false, true);
	}
	else
	{
		m_outer_painter.SetDrawSides(true, true, true, true);
		m_inner_painter.SetDrawSides(true, true, true, true);
	}
}

SDL_FRect node::Dialog::GetXButtonRect() const
{
	return SDL_FRect{ GetSize().w - 35, 1, 34, 28 };
}

SDL_FRect node::Dialog::GetResizeGripRect() const
{
	auto&& rect = GetSize();
	return { rect.w - 20, rect.h - 20, 20, 20 };
}

bool node::Dialog::BeingDragged() const
{
	return !std::holds_alternative<std::monostate>(m_dragData);
}

void node::Dialog::StopDrag()
{
	m_dragData = std::monostate{};
}

node::WidgetSize node::Dialog::CalculateMinSize() const
{
	float min_width = 0;
	float min_height = ControlsMargin + GetTitleBarRect().h;
	if (m_toolbar)
	{
		min_height += ToolBar::height;
	}
	for (const auto& control : m_controls)
	{
		SDL_FRect size_hint = control->GetSizeHint();
		min_height += size_hint.h + ControlsMargin;
		min_width = std::max(min_width, size_hint.w);
	}
	if (m_buttons.size())
	{
		min_height += ButtonHeight + ButtonsMargin;
	}
	else if (m_resizable)
	{
		min_height += ControlsMargin + ButtonsMargin;
	}
	min_width += ControlsMargin * 2; // pad both sides
	float buttons_width = ButtonsMargin; // pad left
	for (const auto& button : m_buttons)
	{
		buttons_width += button->GetSize().w + ButtonsMargin;
	}
	if (m_resizable)
	{
		buttons_width += GetResizeGripRect().w;
	}
	min_width = std::max(min_width, buttons_width);
	min_width = std::max(min_width, MinWidth);
	return { min_width, min_height };
}



node::DialogButton::DialogButton(std::string text, TTF_Font* font, std::function<void()> OnClick,
	const WidgetSize& size, Widget* scene)
	:Widget{ size, scene }, m_text_painter{ font }, m_onClick{ OnClick }
{
	m_text_painter.SetText(std::move(text));
	SetFocusable(true);
}

void node::DialogButton::OnDraw(SDL::Renderer& renderer)
{
	SDL_FRect inner_rect = GetSize().ToRect();
	SDL_Color outline_color = renderer.GetColor(ColorRole::btn_outline);
	SDL_Color inner_color = renderer.GetColor(ColorRole::btn_normal);

	if (b_being_clicked)
	{
		inner_color = renderer.GetColor(ColorRole::btn_outline);
	}
	else if (b_hovered)
	{
		inner_color = renderer.GetColor(ColorRole::btn_hover);
	}

	ThickFilledRoundRect(renderer, inner_rect, 8, 1, outline_color, inner_color, m_outer_painter, m_inner_painter);
	
	SDL_Color text_color = renderer.GetColor(ColorRole::text_normal);
	inner_rect.x += 1;
	inner_rect.y += 1;
	inner_rect.w -= 2;
	inner_rect.h -= 2;
	SDL_FRect text_rect = m_text_painter.GetRect(renderer);
	SDL_FPoint text_start{
		(GetSize().w - text_rect.w) / 2,
		(GetSize().h - text_rect.h) / 2 };
	m_text_painter.Draw(renderer, text_start, text_color);
}

void node::DialogButton::OnMouseOut(MouseHoverEvent&)
{
	b_being_clicked = false;
	b_hovered = false;
}

void node::DialogButton::OnMouseIn(MouseHoverEvent&)
{
	b_hovered = true;
}

MI::ClickEvent node::DialogButton::OnLMBDown(MouseButtonEvent& e)
{
	UNUSED_PARAM(e);
	b_being_clicked = true;
	return MI::ClickEvent::CLICKED;
}

MI::ClickEvent node::DialogButton::OnLMBUp(MouseButtonEvent& e)
{
	UNUSED_PARAM(e);
	if (b_being_clicked)
	{
		b_being_clicked = false;
		m_onClick();
		return MI::ClickEvent::CLICKED;
	}
	return MI::ClickEvent::NONE;
}

node::DialogControl::DialogControl(const WidgetSize& size, Dialog* parent)
	:Widget{ size, parent }, m_size_hint{ 0, 0, size.w, size.h }
{
}
