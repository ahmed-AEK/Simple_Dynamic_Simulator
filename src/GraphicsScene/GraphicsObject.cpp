#include "GraphicsObject.hpp"
#include "IGraphicsScene.hpp"
#include "NodeSDLStylers/SpaceScreenTransformer.hpp"
#include "toolgui/NodeMacros.h"

node::GraphicsObject::GraphicsObject( node::model::Rect sceneRect, ObjectType type, node::IGraphicsScene* scene)
:GraphicsObjectMouseInteractable{ sceneRect }, m_obj_type(type), m_pScene(scene)
{
}

node::GraphicsObject::~GraphicsObject() = default;

void node::GraphicsObject::InvalidateRect()
{
    this->m_pScene->InvalidateRect();
}

void node::GraphicsObject::SetSpaceOrigin(const model::Point& p)
{
    SetSpaceRect({ p.x, p.y, GetRectImpl().w, GetRectImpl().h});
}

void node::GraphicsObject::SetSpaceRect(const model::Rect& rect)
{
    SetRectImpl({
            rect.x,
            rect.y,
            rect.w,
            rect.h
        });
    OnSetSpaceRect(rect);
}

void node::GraphicsObject::OnSetSpaceRect(const model::Rect& rect)
 {
    UNUSED_PARAM(rect);
}

void node::GraphicsObject::OnUpdateRect()
{
}

const node::model::Rect& node::GraphicsObject::GetSpaceRect() const noexcept
{
    return GetRectImpl();
}

node::GraphicsObject* node::GraphicsObject::OnGetInteractableAtPoint(const model::Point& point)
{
    UNUSED_PARAM(point);
    return this;
}

void node::GraphicsObject::UpdateRect()
{
    OnUpdateRect();
}

void node::GraphicsObject::setScene(IGraphicsScene* scene)
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
