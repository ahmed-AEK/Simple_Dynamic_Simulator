#include "Dialog.hpp"
#include "SDL_Framework/SDLCPP.hpp"
#include "Scene.hpp"
#include "Application.hpp"

node::Dialog::Dialog(std::string title, const SDL_Rect& rect, Scene* parent)
	:Widget{ rect, parent }, m_title{ std::move(title) }
{
}

void node::Dialog::Draw(SDL_Renderer* renderer)
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
	SDL_Rect banner_rect = GetTitleBarRect();
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &banner_rect);
	banner_rect.x += 2;
	banner_rect.y += 2;
	banner_rect.w -= 4;
	banner_rect.h -= 4;
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(renderer, &banner_rect);

	DrawTitle(renderer, SDL_Point{ banner_rect.x + 5, banner_rect.y });
	DrawXButton(renderer, GetXButtonRect());
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

	if (!b_being_dragged)
	{
		return;
	}

	int x_distance = current_mouse_point.x - m_drag_mouse_start_position.x;
	int y_distance = current_mouse_point.y - m_drag_mouse_start_position.y;
	SDL_Rect new_rect{ m_drag_edge_start_position.x + x_distance, m_drag_edge_start_position.y + y_distance , GetRect().w, GetRect().h};
	if (new_rect.x + new_rect.w > GetScene()->GetRect().w)
	{
		new_rect.x = GetScene()->GetRect().w - new_rect.w;
	}
	if (new_rect.x < 0)
	{
		new_rect.x = 0;
	}
	if (new_rect.y + new_rect.h > GetScene()->GetRect().h)
	{
		new_rect.y = GetScene()->GetRect().h - new_rect.h;
	}
	if (new_rect.y < 0)
	{
		new_rect.y = 0;
	}

	SetRect(new_rect);
}

MI::ClickEvent node::Dialog::OnLMBDown(const SDL_Point& current_mouse_point)
{
	const auto& X_btn_rect = GetXButtonRect();
	if (SDL_PointInRect(&current_mouse_point, &X_btn_rect))
	{
		b_being_closed = true;
		return MI::ClickEvent::CLICKED;
	}

	const auto& banner_rect = GetTitleBarRect();
	if (SDL_PointInRect(&current_mouse_point, &banner_rect))
	{
		m_drag_edge_start_position = SDL_Point{ GetRect().x, GetRect().y };
		m_drag_mouse_start_position = current_mouse_point;
		b_being_dragged = true;
		return MI::ClickEvent::CAPTURE_START;
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
			GetScene()->PopDialog(this);
			return MI::ClickEvent::CLICKED;
		}
	}
	if (b_being_dragged)
	{
		b_being_dragged = false;
		return MI::ClickEvent::CAPTURE_END;
	}
	return MI::ClickEvent::NONE;
}

void node::Dialog::OnMouseOut()
{
	b_being_closed = false;
}

void node::Dialog::DrawTitle(SDL_Renderer* renderer, const SDL_Point& start )
{
	SDL_Color Black = { 50, 50, 50, 255 };
	auto textSurface = SDLSurface{ TTF_RenderText_Solid(GetScene()->GetApp()->getFont().get(), m_title.c_str(), Black) };
	auto textTexture = SDLTexture{ SDL_CreateTextureFromSurface(renderer, textSurface.get()) };

	SDL_Rect text_rect{};
	SDL_QueryTexture(textTexture.get(), NULL, NULL, &text_rect.w, &text_rect.h);
	text_rect.x = start.x;
	text_rect.y = start.y + 40 / 2 - text_rect.h / 2;
	SDL_RenderCopy(renderer, textTexture.get(), NULL, &text_rect);
}

void node::Dialog::DrawXButton(SDL_Renderer* renderer, const SDL_Rect& rect)
{
	SDL_Rect base = rect;
	SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
	SDL_RenderFillRect(renderer, &base);
	base.x += 2;
	base.y += 2;
	base.w -= 4;
	base.h -= 4;
	if (b_being_closed)
	{
		SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
	}
	else if (b_mouse_on_close)
	{
		SDL_SetRenderDrawColor(renderer, 255, 60, 60, 255);
	}
	else
	{
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	}
	SDL_RenderFillRect(renderer, &base);

	const int dist_from_side = 8;
	if (b_mouse_on_close)
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

SDL_Rect node::Dialog::GetTitleBarRect() const
{
	const auto& this_rect = GetRect();
	return SDL_Rect{ this_rect.x, this_rect.y, this_rect.w, 40 };
}

SDL_Rect node::Dialog::GetXButtonRect() const
{
	const auto& this_rect = GetRect();
	return SDL_Rect{ this_rect.x + this_rect.w - 40, this_rect.y + 5, 30, 30 };
}
