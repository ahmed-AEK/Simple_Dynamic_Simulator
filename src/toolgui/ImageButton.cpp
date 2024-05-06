
#include "ImageButton.h"
#include <SDL_Framework/SDL_headers.h>
#include "toolgui/NodeMacros.h"

ImageButton::ImageButton(const std::string_view img_path, std::function<void(void)>&& fn):
m_source_path(img_path), m_OnClick(fn)
{
}

void ImageButton::Draw(SDL_Renderer *renderer)
{
    if (!m_texture_loaded)
    {
        m_btn_tex.reset(SDL_CreateTextureFromSurface(renderer, m_btn_surface.get()));
        m_texture_loaded = true;
    }
    SDL_RenderCopy(renderer, m_btn_tex.get(), NULL, &m_rect);
}

const SDL_Rect &ImageButton::GetRect() const noexcept
{
    return m_rect;
}

void ImageButton::SetRect(const SDL_Rect &rect)
{
    this->m_rect = rect;
}

void ImageButton::loadimage()
{
    m_btn_surface.reset(IMG_Load(m_source_path.c_str()));
    m_texture_loaded = false;
    if (!m_btn_surface)
    {
        SDL_Log("failed to load \"%s\".\n", m_source_path.c_str());
        throw std::runtime_error("failed to load \""+ m_source_path + "\"");
    }
}

void ImageButton::Bind(std::function<void(void)> &&fn)
{
    m_OnClick = fn;
}

const std::string_view ImageButton::GetPath() const
{
    return this->m_source_path;
}

ImageButton* ImageButton::OnGetInteractableAtPoint(const SDL_Point &point)
{
    if (SDL_PointInRect(&point, &m_rect))
    {
        return this;
    }
    else
    {
        return nullptr;
    }
}

MI::ClickEvent ImageButton::OnLMBUp(const SDL_Point &current_mouse_point)
{
    UNUSED_PARAM(current_mouse_point);
    if (m_being_clicked)
    {
        m_OnClick();
    }
    m_being_clicked = false;
    return MI::ClickEvent::CLICKED;
}

void ImageButton::OnMouseOut()
{
    m_being_clicked = false;
}

MI::ClickEvent ImageButton::OnLMBDown(const SDL_Point &current_mouse_point)
{
    UNUSED_PARAM(current_mouse_point);
    m_being_clicked = true;
    return MI::ClickEvent::NONE;
}
