#pragma once

#include "GraphicsScene/GraphicsScene_exports.h"

#include "toolgui/MouseInteractable.hpp"
#include "toolgui/Handle.hpp"
#include "NodeModels/Utils.hpp"

namespace node
{

enum class ObjectType
{
    block,
    socket,
    netNode,
    net,
    interactive,
    OBJECTTYPE_COUNT
};

class IGraphicsScene;

class GraphicsObject;
using GraphicsObjectMouseInteractable = MI::MouseInteractable<GraphicsObject, model::Rect, model::Point>;

class GRAPHICSSCENE_API GraphicsObject: protected GraphicsObjectMouseInteractable
{
    friend GraphicsObjectMouseInteractable;
public:
    GraphicsObject(model::Rect sceneRect, ObjectType type, IGraphicsScene* scene);
    virtual ~GraphicsObject();

    virtual void Draw(SDL_Renderer* renderer) = 0;

    ObjectType GetObjectType() const { return m_obj_type; };

    const model::Rect& GetSpaceRect() const noexcept;
    void InvalidateRect();
    void SetSpaceRect(const model::Rect& rect);
    void SetSpaceOrigin(const model::Point& p);
    virtual void UpdateRect();

    void setScene(IGraphicsScene* scene);
    node::HandlePtr<GraphicsObject> GetFocusHandlePtr();
    IGraphicsScene* GetScene() const { return m_pScene; }

    bool isSelectable() const { return b_selectable; }
    bool isDraggable() const { return b_draggable; }
    bool isAligned() const { return b_aligned; }

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

    bool b_selectable = true;
    bool b_draggable = true;
    bool b_aligned = true;
private:
    ObjectType m_obj_type;
    IGraphicsScene* m_pScene;
    node::HandleOwnigPtr<GraphicsObject> m_focusHandle = node::HandleAllocator<GraphicsObject>::CreateHandle(this); 
};

}