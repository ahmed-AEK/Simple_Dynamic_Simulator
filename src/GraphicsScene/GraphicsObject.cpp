#include "GraphicsObject.hpp"
#include "GraphicsScene.hpp"
#include "toolgui/NodeMacros.h"

node::GraphicsObject::GraphicsObject(SDL_Rect sceneRect, ObjectType type, node::GraphicsScene* scene)
:m_obj_type(type), m_pScene(scene), m_spaceRect{sceneRect}
{
    if (scene)
    {
        SetRectImpl(m_pScene->SpaceToScreenRect(m_spaceRect));
    }
}

node::GraphicsObject::~GraphicsObject() = default;

void node::GraphicsObject::InvalidateRect()
{
    this->m_pScene->InvalidateRect();
}

void node::GraphicsObject::SetSpaceOrigin(const SDL_Point& p)
{
    SetSpaceRect({ p.x, p.y, m_spaceRect.w, m_spaceRect.h });
}

void node::GraphicsObject::SetSpaceRect(const SDL_Rect& rect)
{
    m_spaceRect = rect;
    if (m_pScene) {
        const SDL_Point p1 = m_pScene->SpaceToScreenPoint({ rect.x, rect.y });
        const SDL_Point p2 = m_pScene->SpaceToScreenVector({ rect.w, rect.h });
        SetRectImpl({
            p1.x,
            p1.y,
            p2.x,
            p2.y
            });
    }
    OnSetSpaceRect(rect);
}

void node::GraphicsObject::OnSetSpaceRect(const SDL_Rect& rect)
{
    UNUSED_PARAM(rect);
}

void node::GraphicsObject::OnUpdateRect()
{
}

const SDL_Rect& node::GraphicsObject::GetSpaceRect() const noexcept
{
    return m_spaceRect;
}

node::GraphicsObject* node::GraphicsObject::OnGetInteractableAtPoint(const SDL_Point& point)
{
    UNUSED_PARAM(point);
    return this;
}

void node::GraphicsObject::UpdateRect()
{
    SetRectImpl(this->m_pScene->SpaceToScreenRect(m_spaceRect));
    OnUpdateRect();
}

void node::GraphicsObject::setScene(GraphicsScene* scene)
{
    m_pScene = scene;
    if (scene)
    {
        UpdateRect();
    }
}

node::HandlePtr<node::GraphicsObject> node::GraphicsObject::GetFocusHandlePtr()
{
    return GetMIHandlePtr();
}
