#include "GraphicsObject.hpp"
#include "GraphicsScene.hpp"
#include "toolgui/MouseInteractable.hpp"

template class MI::MouseInteractable<node::GraphicsObject, node::model::ObjectSize, node::model::Point>;

node::GraphicsObject::GraphicsObject(const node::model::ObjectSize& size, ObjectType type, node::GraphicsObject* parent)
:GraphicsObjectMouseInteractable{}, m_parent{parent}, m_size{size}, m_obj_type(type)
{
    if (m_parent)
    {
        m_parent->m_children.push_back(this);
        SetScene(m_parent->GetScene());
    }
}

node::GraphicsObject::~GraphicsObject()
{
    UnParent();
}


void node::GraphicsObject::SetParent(GraphicsObject* parent)
{
    if (m_parent && m_parent != parent)
    {
        UnParent();
    }
    if (parent && parent != m_parent)
    {
        m_parent = parent;
        m_parent->m_children.push_back(this);
        SetScene(m_parent->GetScene());
    }
}

void node::GraphicsObject::UnParent()
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
        SetScene(nullptr);
    }
}


void node::GraphicsObject::InvalidateRect()
{
}

void node::GraphicsObject::OnSetPosition(const model::Point& position)
{
    m_position = position;
}

void node::GraphicsObject::OnSetSize(const model::ObjectSize& size)
{
    m_size = size;
}

node::model::Point node::GraphicsObject::GetScenePosition() const
{
    model::Point result = GetPosition();
    GraphicsObject* parent = m_parent;
    while (parent)
    {
        result = result + parent->GetPosition();
        parent = parent->m_parent;
    }
    return result;
}

void node::GraphicsObject::SetPosition(const model::Point& position)
{
    OnSetPosition(position);
}

void node::GraphicsObject::SetSize(const model::ObjectSize& size)
{
    OnSetSize(size);
}

node::model::Rect node::GraphicsObject::GetSceneRect() const
{
    model::Rect rect = GetSize().ToRect();
    const auto& pos = GetScenePosition();
    rect.x = pos.x;
    rect.y = pos.y;
    return rect;
}

node::GraphicsObject* node::GraphicsObject::OnGetInteractableAtPoint(const model::Point& point)
{
    UNUSED_PARAM(point);
    return this;
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

    for (auto* child : m_children)
    {
        child->SetScene(scene);
    }
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
