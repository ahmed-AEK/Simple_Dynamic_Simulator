#pragma once

#include "GraphicsScene/GraphicsScene_exports.h"

#include "toolgui/MouseInteractable_interface.hpp"
#include "toolgui/Handle.hpp"
#include "NodeModels/Utils.hpp"


namespace node
{
    class GraphicsObject;
}

extern template class MI::MouseInteractable<node::GraphicsObject, node::model::Rect, node::model::Point>;


namespace MI
{
    template<>
    struct MouseHoverEvent<node::GraphicsObject>
    {
        node::model::Point point;
    };

    template<>
    struct MouseButtonEvent<node::GraphicsObject>
    {
        node::model::Point point;
    };
}

namespace node
{

enum class ObjectType
{
    block,
    socket,
    netNode,
    netSegment,
    interactive,
    OBJECTTYPE_COUNT
};

class GraphicsScene;

class GraphicsObject;

using GraphicsObjectMouseInteractable = MI::MouseInteractable<GraphicsObject, model::Rect, model::Point>;
class SpaceScreenTransformer;

class GRAPHICSSCENE_API GraphicsObject: protected GraphicsObjectMouseInteractable
{
    friend GraphicsObjectMouseInteractable;
public:
    GraphicsObject(model::Rect sceneRect, ObjectType type, GraphicsScene* scene = nullptr);
    virtual ~GraphicsObject();

    virtual void Draw(SDL_Renderer* renderer, const SpaceScreenTransformer& transformer) = 0;

    ObjectType GetObjectType() const { return m_obj_type; };

    const model::Rect& GetSpaceRect() const noexcept;
    void InvalidateRect();
    void SetSpaceRect(const model::Rect& rect);
    void SetSpaceOrigin(const model::Point& p);
    virtual void UpdateRect();

    node::HandlePtr<GraphicsObject> GetFocusHandlePtr();
    GraphicsScene* GetScene() const { return m_pScene; }
    void SetScene(GraphicsScene* scene);

    void SetSelected(bool value);
    bool IsSelected() const { return b_selected; }
    bool isSelectable() const { return b_selectable; }
    bool isDraggable() const { return b_draggable; }
    bool isAligned() const { return b_aligned; }
    void SetVisible(bool visible = true) { b_isVisible = visible; };
    bool IsVisible() const { return b_isVisible; }
    using GraphicsObjectMouseInteractable::GetInteractableAtPoint;
    using GraphicsObjectMouseInteractable::MouseOut;
    using GraphicsObjectMouseInteractable::MouseIn;
    using GraphicsObjectMouseInteractable::MouseMove;
    using GraphicsObjectMouseInteractable::LMBDown;
    using GraphicsObjectMouseInteractable::LMBUp;
    using GraphicsObjectMouseInteractable::RMBDown;
    using GraphicsObjectMouseInteractable::RMBUp;
    using GraphicsObjectMouseInteractable::GetMIHandlePtr;
protected:
    virtual GraphicsObject* OnGetInteractableAtPoint(const model::Point& point) override;
    virtual void OnSetSpaceRect(const model::Rect& rect);
    virtual void OnUpdateRect();
    virtual void OnSelectChange() {}
    virtual void OnSceneChange() {}

    bool b_selectable = true;
    bool b_draggable = true;
    bool b_aligned = true;
    bool b_isVisible = true;
private:
    bool b_selected = false;
    ObjectType m_obj_type;
    GraphicsScene* m_pScene;
    node::HandleOwnigPtr<GraphicsObject> m_focusHandle = node::HandleAllocator<GraphicsObject>::CreateHandle(this); 
};

}