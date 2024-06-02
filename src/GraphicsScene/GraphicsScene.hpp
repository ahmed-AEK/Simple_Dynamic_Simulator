#pragma once

#include "GraphicsScene/GraphicsScene_exports.h"
#include "toolgui/Widget.hpp"
#include "GraphicsScene/SpaceScreenTransformer.hpp"
#include "GraphicsScene/IGraphicsScene.hpp"
#include <vector>
#include <span>

namespace node
{

class Node;
class NodeSocket;
class GraphicsObject;
class GraphicsLogic;
class Scene;

struct GRAPHICSSCENE_API ObjectSlot
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

struct GRAPHICSSCENE_API DragObject
{
    HandlePtr<GraphicsObject> m_object;
    SDL_Point m_start_position;
};

class GRAPHICSSCENE_API GraphicsScene: public node::Widget, public node::IGraphicsScene
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

    void SetScrollRatio(double scroll_ratio) { m_scroll_ratio = scroll_ratio; }
    double GetScrollRatio() const noexcept { return m_scroll_ratio; }
    double GetZoomScale() const { return m_zoomScale; }

    void AddObject(std::unique_ptr<node::GraphicsObject> obj, int z_order);
    std::unique_ptr<node::GraphicsObject> PopObject(const node::GraphicsObject* obj);

    void UpdateObjectsRect();

    void SetSpaceRect(const SDL_Rect& rect);
    const SDL_Rect& GetSpaceRect() const noexcept;
    const SDL_Rect& GetSpaceRectBase() const noexcept;
    void InvalidateRect() override;

    std::span<const HandlePtr<GraphicsObject>> GetCurrentSelection() const;
    std::span<HandlePtr<GraphicsObject>> GetCurrentSelection();
    void AddSelection(HandlePtr<GraphicsObject> handle);
    bool IsObjectSelected(const GraphicsObject& obj) const override;
    void ClearCurrentSelection();

    const SpaceScreenTransformer& GetSpaceScreenTransformer() const override;
    SDL_Point QuantizePoint(const SDL_Point& p);

    std::vector<Node*> GetNodes();

    virtual void Draw(SDL_Renderer* renderer) override;

    void SetCurrentHover(GraphicsObject* current_hover);
    bool IsMouseCaptured() const { return m_mouse_capture_mode != CAPTURE_MODE::NONE; }
    NodeSocket* GetSocketAt(const SDL_Point space_point);

    void SetMode(GraphicsSceneMode value) { m_SceneMode = value; }
    GraphicsSceneMode GetMode() const { return m_SceneMode; }

    void SetGraphicsLogic(std::unique_ptr<GraphicsLogic> logic);
    GraphicsLogic* GetGraphicsLogic() { return m_graphicsLogic.get(); }
    IGraphicsSceneController* GetController() const override;

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
    std::unique_ptr<GraphicsLogic> m_graphicsLogic;
    SpaceScreenTransformer m_spaceScreenTransformer;
    CAPTURE_MODE m_mouse_capture_mode = CAPTURE_MODE::NONE;
    GraphicsSceneMode m_SceneMode = GraphicsSceneMode::Normal;
};

}