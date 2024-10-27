#include "GraphicsObject.hpp"
#include "NodeSDLStylers/SpaceScreenTransformer.hpp"
#include "toolgui/NodeMacros.h"
#include "toolgui/MouseInteractable.hpp"
#include "GraphicsScene.hpp"

template class MI::MouseInteractable<node::GraphicsObject, node::model::Rect, node::model::Point>;

node::GraphicsObject::GraphicsObject( node::model::Rect sceneRect, ObjectType type, node::GraphicsScene* scene)
:GraphicsObjectMouseInteractable{ sceneRect }, m_obj_type(type), m_pScene(scene)
{
}

node::GraphicsObject::~GraphicsObject() = default;

void node::GraphicsObject::InvalidateRect()
{
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

node::HandlePtr<node::GraphicsObject> node::GraphicsObject::GetFocusHandlePtr()
{
    return GetMIHandlePtr();
}

void node::GraphicsObject::SetScene(GraphicsScene* scene)
{
    if (scene == GetScene())
    {
        return;
    }

    m_pScene = scene;
    OnSceneChange();
}

void node::GraphicsObject::SetSelected(bool value)
{
    bool old_value = b_selected;
    b_selected = value;
    if (old_value != value)
    {
        OnSelectChange();
    }
}
