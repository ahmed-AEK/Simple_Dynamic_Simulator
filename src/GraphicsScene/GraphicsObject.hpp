#pragma once

#include "GraphicsScene/GraphicsScene_exports.h"

#include "toolgui/MouseInteractable.hpp"
#include "toolgui/Handle.hpp"

namespace node
{

enum class ObjectType
{
    node,
    socket,
    netNode,
    net,
    interactive,
    logic,
    OBJECTTYPE_COUNT
};

class GraphicsScene;
class GRAPHICSSCENE_API GraphicsObject: public MI::MouseInteractable<GraphicsObject>
{
public:
    GraphicsObject(SDL_Rect sceneRect, ObjectType type, GraphicsScene* scene);
    virtual void Draw(SDL_Renderer* renderer) = 0;
    ObjectType GetObjectType() const { return m_obj_type; };
    const SDL_Rect& GetSpaceRect() const noexcept;
    void InvalidateRect();
    void SetSpaceRect(const SDL_Rect& rect);
    void SetSpaceOrigin(const SDL_Point& p);
    virtual ~GraphicsObject() noexcept {};
    virtual void UpdateRect();
    void setScene(GraphicsScene* scene);
    node::HandlePtr<GraphicsObject> GetFocusHandlePtr();
    bool isSelectable() const { return b_selectable; }
    bool isDraggable() const { return b_draggable; }
    bool isAligned() const { return b_aligned; }
    const GraphicsScene* GetScene() const { return m_pScene; }
    GraphicsScene* GetScene() { return m_pScene; }
protected:
    virtual GraphicsObject* OnGetInteractableAtPoint(const SDL_Point& point) override;
    virtual void OnSetSpaceRect(const SDL_Rect& rect);
    virtual void OnUpdateRect();
    bool b_selectable = true;
    bool b_draggable = true;
    bool b_aligned = true;
private:
    ObjectType m_obj_type;
    GraphicsScene* m_pScene;
    SDL_Rect m_spaceRect;
    node::HandleOwnigPtr<GraphicsObject> m_focusHandle = node::HandleAllocator<GraphicsObject>::CreateHandle(this); 
};

}