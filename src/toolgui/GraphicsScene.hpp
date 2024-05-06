#pragma once

#include "toolgui/toolgui_exports.h"

#include "toolgui/Scene.hpp"
#include "toolgui/GraphicsObject.hpp"
#include <vector>
#include <span>

namespace node
{

class Node;
class NodeSocket;

struct TOOLGUI_API ObjectSlot
{
    std::unique_ptr<node::GraphicsObject> m_ptr;
    int z_order;
};
enum class GraphicsSceneMode
{
    Normal,
    Insert,
    Delete,
};

struct TOOLGUI_API DragObject
{
    HandlePtr<GraphicsObject> m_object;
    SDL_Point m_start_position;
};

class TOOLGUI_API GraphicsScene: public node::Widget
{
public:
    enum class CAPTURE_MODE
    {
        NONE,
        OBJECT,
        OBJECTS_DRAG,
        SCREEN_DRAG
    };


    GraphicsScene(SDL_Rect rect, node::Scene* parent);
    void SetScrollRatio(double scroll_ratio);
    double GetScrollRatio() const noexcept;
    double GetZoomScale() const { return m_zoomScale; }
    void AddObject(std::unique_ptr<node::GraphicsObject> obj, int z_order);
    std::unique_ptr<node::GraphicsObject> PopObject(node::GraphicsObject* obj);
    void UpdateObjectsRect();
    void SetSpaceRect(const SDL_Rect& rect);
    const SDL_Rect& GetSpaceRect() const noexcept;
    const SDL_Rect& GetSpaceRectBase() const noexcept;
    std::span<const HandlePtr<GraphicsObject>> GetCurrentSelection() const;
    std::span<HandlePtr<GraphicsObject>> GetCurrentSelection();
    void AddSelection(HandlePtr<GraphicsObject> handle);
    bool isObjectSelected(const GraphicsObject& obj) const;
    void ClearCurrentSelection();
    SDL_Point ScreenToSpacePoint(const SDL_Point& p) const noexcept;
    SDL_Point SpaceToScreenPoint(const SDL_Point& p) const noexcept;
    SDL_Point ScreenToSpaceVector(const SDL_Point& p) const noexcept;
    SDL_Point SpaceToScreenVector(const SDL_Point& p) const noexcept;
    SDL_Rect ScreenToSpaceRect(const SDL_Rect& rect) const noexcept;
    SDL_Rect SpaceToScreenRect(const SDL_Rect& rect) const noexcept;
    SDL_Point QuantizePoint(const SDL_Point& p);
    std::vector<Node*> GetNodes();
    virtual void Draw(SDL_Renderer* renderer) override;
    void SetCurrentHover(GraphicsObject* current_hover);
    bool IsMouseCaptured() const { return m_mouse_capture_mode != CAPTURE_MODE::NONE; }
    NodeSocket* GetSocketAt(const SDL_Point space_point);
    void SetMode(GraphicsSceneMode value) { m_SceneMode = value; }
    GraphicsSceneMode GetMode() const { return m_SceneMode; }
protected:
    virtual void OnSetRect(const SDL_Rect& rect) override;
    virtual void OnMouseMove(const SDL_Point& p) override;
    virtual MI::ClickEvent OnLMBDown(const SDL_Point& p) override;
    virtual MI::ClickEvent OnLMBUp(const SDL_Point& p) override;
    virtual bool OnScroll(const double amount, const SDL_Point& p) override;
    virtual node::GraphicsObject* GetInteractableAt(const SDL_Point& p) const;
private:
    bool InternalSelectObject(GraphicsObject* object);
    SDL_Rect m_spaceRect_base;
    SDL_Rect m_spaceRect;
    double m_scroll_ratio = 1.25;
    double m_zoomScale;
    int m_spaceQuantization = 20;
    SDL_Point m_StartPointScreen{ 0,0 };
    SDL_Point m_startEdgeSpace{ 0,0 };
    std::vector<ObjectSlot> m_objects;
    HandlePtr<GraphicsObject> m_current_mouse_hover;
    std::vector<HandlePtr<GraphicsObject>> m_current_selection;
    std::vector<DragObject> m_drag_objects;
    CAPTURE_MODE m_mouse_capture_mode = CAPTURE_MODE::NONE;
    GraphicsSceneMode m_SceneMode = GraphicsSceneMode::Normal;
};

}