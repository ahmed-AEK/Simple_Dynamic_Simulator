#include "Widget.hpp"
#include "Scene.hpp"
#include "toolgui/ContextMenu.hpp"
#include "toolgui/MouseInteractable.hpp"
#include "SDL_Framework/SDL_Math.hpp"
#include "SDL_Framework/SDLRenderer.hpp"

#include <ranges>
#include <algorithm>

template class MI::MouseInteractable<node::Widget, node::WidgetSize, SDL_FPoint>;

void node::Widget::OnSetPosition(const SDL_FPoint& pos)
{
    m_position = pos;
}

void node::Widget::OnSetSize(const WidgetSize& size)
{
    m_size = size;
}

bool node::Widget::OnScroll(const double amount, const SDL_FPoint& p)
{
    UNUSED_PARAM(amount);
    UNUSED_PARAM(p);
    return false;
}

node::Widget* node::Widget::GetFocusable()
{
    if (m_focus_proxy)
    {
        return m_focus_proxy->GetFocusable();
    }
    if (IsFocusable())
    {
        return this;
    }
    return nullptr;
}

node::Application* node::Widget::GetApp() const
{
    if (const auto* parent = GetParent())
    {
        return parent->GetApp();
    }
    return nullptr;
}

void node::Widget::SetParent(Widget* parent)
{
    if (m_parent && m_parent != parent)
    {
        UnParent();
    }
    if (parent && parent != m_parent)
    {
        m_parent = parent;
        m_parent->m_children.push_back(this);
    }
}

void node::Widget::UnParent()
{
    if (m_parent)
    {
        auto& other_children = m_parent->m_children;
        auto it = std::find(other_children.begin(), other_children.end(), this);
        assert(it != other_children.end());
        if (it != other_children.end())
        {
            other_children.erase(it);
        }
        m_parent = nullptr;
    }
}

void node::Widget::SetFocusProxy(Widget* other)
{
    if (!other)
    {
        m_focus_proxy = nullptr;
        return;
    }
    m_focus_proxy = other->GetMIHandlePtr();
}

SDL_FPoint node::Widget::GetGlobalPosition() const
{
    SDL_FPoint pos = GetPosition();
    Widget* parent = GetParent();
    while (parent)
    {
        pos = pos + parent->GetPosition();
        parent = parent->GetParent();
    }
    return pos;
}

node::Widget::~Widget()
{
    UnParent();
}

node::Widget::Widget(const WidgetSize& size, Widget* parent)
:m_parent(parent), m_size{ size }
{
    if (m_parent)
    {
        m_parent->m_children.push_back(this);
    }
}

void node::Widget::SetPosition(const SDL_FPoint& pos)
{
    OnSetPosition(pos);
}

void node::Widget::SetSize(const WidgetSize& size)
{
    OnSetSize(size);
}

void node::Widget::Draw(SDL::Renderer& renderer)
{
    auto palette_scope = renderer.SetColorPalette(m_color_palette);
    OnDraw(renderer);
    for (auto* child : m_children)
    {
        auto clip = renderer.ClipRect(WidgetRect(*child));
        if (clip)
        {
            child->Draw(renderer);
        }
    }
}

void node::Widget::OnDraw(SDL::Renderer& renderer)
{
    UNUSED_PARAM(renderer);
}

node::Widget* node::Widget::OnGetInteractableAtPoint(const SDL_FPoint &point)
{
    for (auto* child : std::ranges::reverse_view{ m_children })
    {
        if (auto* obj = child->GetInteractableAtPoint(point - child->GetPosition()))
        {
            return obj;
        }
    }
    return this;
}

void node::Widget::NotifyParentPaletteUpdate(const ColorPalette& palette)
{
    if (m_color_palette.empty())
    {
        OnPaletteChanged(palette);
        NotifyPaletteUpdate(palette);
    }
    else
    {
        OnParentPaletteChanged(palette);
    }
}

void node::Widget::NotifyPaletteUpdate(const ColorPalette& palette)
{
    for (auto* child : m_children)
    {
        child->NotifyParentPaletteUpdate(palette);
    }
}

void node::Widget::SetColorPalette(ColorPalette color_palette)
{
    if (color_palette != m_color_palette)
    {
        m_color_palette = std::move(color_palette);
        OnPaletteChanged(m_color_palette);
        NotifyPaletteUpdate(m_color_palette);
    }
}