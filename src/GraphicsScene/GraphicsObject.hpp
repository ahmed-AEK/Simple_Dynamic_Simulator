#pragma once

#include "GraphicsScene/GraphicsScene_exports.h"

#include "toolgui/MouseInteractable_interface.hpp"
#include "toolgui/Handle.hpp"
#include "NodeModels/Utils.hpp"


namespace node
{
    class GraphicsObject;
    namespace model
    {
        struct ObjectSize
        {
            model::node_int w;
            model::node_int h;

            model::Rect ToRect() const
            {
                return model::Rect{ 0,0,w,h };
            }
        };
    }
}

extern template class MI::MouseInteractable<node::GraphicsObject, node::model::ObjectSize, node::model::Point>;


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

namespace SDL
{
    class Renderer;
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

using GraphicsObjectMouseInteractable = MI::MouseInteractable<GraphicsObject, model::ObjectSize, model::Point>;
class SpaceScreenTransformer;

class GRAPHICSSCENE_API GraphicsObject: protected GraphicsObjectMouseInteractable
{
    friend GraphicsObjectMouseInteractable;
public:
    GraphicsObject(const node::model::ObjectSize& size, ObjectType type, GraphicsObject* parent = nullptr);
    virtual ~GraphicsObject();

    virtual void Draw(SDL::Renderer& renderer, const SpaceScreenTransformer& transformer) = 0;

    ObjectType GetObjectType() const { return m_obj_type; };

    void InvalidateRect();

    node::HandlePtr<GraphicsObject> GetFocusHandlePtr();

    void SetSelected(bool value);
    void SetAligned(bool value) { b_aligned = value; }
    void SetSelectable(bool value) { b_selectable = value; }
    bool IsSelected() const { return b_selected; }
    bool isSelectable() const { return b_selectable; }
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

    model::Point GetPosition() const { return m_position; }
    model::Point GetScenePosition() const;
    model::ObjectSize GetSize() const { return m_size; }
    void SetPosition(const model::Point& position);
    void SetSize(const model::ObjectSize& size);

    void SetParent(GraphicsObject* parent);
    void UnParent();
    GraphicsScene* GetScene() const { return m_pScene; }

    model::Rect GetSceneRect() const;
protected:
    virtual GraphicsObject* OnGetInteractableAtPoint(const model::Point& point) override;
    virtual void OnSelectChange() {}
    virtual void OnSetPosition(const model::Point& position);
    virtual void OnSetSize(const model::ObjectSize& size);

private:
    friend class GraphicsScene;
    void SetScene(GraphicsScene* scene);

    GraphicsScene* m_pScene = nullptr;
    GraphicsObject* m_parent = nullptr;
    std::vector<GraphicsObject*> m_children;
    bool b_selectable = true;
    bool b_aligned = true;
    bool b_isVisible = true;
    bool b_selected = false;
    model::Point m_position{};
    model::ObjectSize m_size{};
    ObjectType m_obj_type;
    node::HandleOwnigPtr<GraphicsObject> m_focusHandle = node::HandleAllocator<GraphicsObject>::CreateHandle(this); 
};

}