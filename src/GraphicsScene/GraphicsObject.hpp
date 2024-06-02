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
    OBJECTTYPE_COUNT
};

class IGraphicsScene;

class GRAPHICSSCENE_API GraphicsObject: public MI::MouseInteractable<GraphicsObject>
{
public:
    GraphicsObject(SDL_Rect sceneRect, ObjectType type, IGraphicsScene* scene);
    virtual ~GraphicsObject();

    virtual void Draw(SDL_Renderer* renderer) = 0;

    ObjectType GetObjectType() const { return m_obj_type; };

    const SDL_Rect& GetSpaceRect() const noexcept;
    void InvalidateRect();
    void SetSpaceRect(const SDL_Rect& rect);
    void SetSpaceOrigin(const SDL_Point& p);
    virtual void UpdateRect();

    void setScene(IGraphicsScene* scene);
    node::HandlePtr<GraphicsObject> GetFocusHandlePtr();
    const IGraphicsScene* GetScene() const { return m_pScene; }
    IGraphicsScene* GetScene() { return m_pScene; }

    bool isSelectable() const { return b_selectable; }
    bool isDraggable() const { return b_draggable; }
    bool isAligned() const { return b_aligned; }


protected:
    virtual GraphicsObject* OnGetInteractableAtPoint(const SDL_Point& point) override;
    virtual void OnSetSpaceRect(const SDL_Rect& rect);
    virtual void OnUpdateRect();

    bool b_selectable = true;
    bool b_draggable = true;
    bool b_aligned = true;
private:
    ObjectType m_obj_type;
    IGraphicsScene* m_pScene;
    SDL_Rect m_spaceRect;
    node::HandleOwnigPtr<GraphicsObject> m_focusHandle = node::HandleAllocator<GraphicsObject>::CreateHandle(this); 
};

}