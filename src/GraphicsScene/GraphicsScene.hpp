#pragma once

#include "GraphicsScene/GraphicsScene_exports.h"
#include "toolgui/Widget.hpp"
#include "NodeSDLStylers/SpaceScreenTransformer.hpp"
#include "GraphicsScene/IGraphicsScene.hpp"
#include <vector>
#include <span>
#include "GraphicsScene/tools/GraphicsTool.hpp"
#include <optional>
#include "NodeSDLStylers/BlockStyler.hpp"
#include "NodeModels/SceneModelManager.hpp"

namespace node
{

class BlockObject;
class BlockSocketObject;
class GraphicsObject;
class GraphicsLogic;
class IGraphicsSceneController;
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

class GRAPHICSSCENE_API GraphicsScene: public node::Widget, public node::IGraphicsScene, public node::SingleObserver<SceneModification>
{
public:
    enum class CAPTURE_MODE
    {
        NONE,
        OBJECT,
        OBJECTS_DRAG,
        SCREEN_DRAG
    };


    GraphicsScene(const SDL_Rect& rect, node::Scene* parent);
    ~GraphicsScene() override;

    void SetScrollRatio(double scroll_ratio) { m_scroll_ratio = scroll_ratio; }
    double GetScrollRatio() const noexcept { return m_scroll_ratio; }
    double GetZoomScale() const { return m_zoomScale; }

    void AddObject(std::unique_ptr<node::GraphicsObject> obj, int z_order);
    void SetSceneModel(std::shared_ptr<SceneModelManager> scene);
    std::shared_ptr<SceneModelManager> GetSceneModel() { return m_sceneModel; }
    std::unique_ptr<node::GraphicsObject> PopObject(const node::GraphicsObject* obj);

    void UpdateObjectsRect();

    void SetSpaceRect(const model::Rect& rect);
    const model::Rect& GetSpaceRect() const noexcept;
    const model::Rect& GetSpaceRectBase() const noexcept;
    void InvalidateRect() override;

    std::span<const HandlePtr<GraphicsObject>> GetCurrentSelection() const;
    std::span<HandlePtr<GraphicsObject>> GetCurrentSelection();
    void AddSelection(HandlePtr<GraphicsObject> handle);
    bool IsObjectSelected(const GraphicsObject& obj) const override;
    void ClearCurrentSelection();

    const SpaceScreenTransformer& GetSpaceScreenTransformer() const override;
    model::Point QuantizePoint(const model::Point& p);

    std::vector<BlockObject*> GetNodes();

    virtual void Draw(SDL_Renderer* renderer) override;

    GraphicsObject* GetCurrentHover() noexcept { return m_current_mouse_hover.GetObjectPtr(); }
    void SetCurrentHover(GraphicsObject* current_hover);
    bool IsMouseCaptured() const { return m_mouse_capture_mode != CAPTURE_MODE::NONE; }
    BlockSocketObject* GetSocketAt(const model::Point space_point);

    void SetMode(GraphicsSceneMode value) { m_SceneMode = value; }
    GraphicsSceneMode GetMode() const { return m_SceneMode; }

    void SetGraphicsLogic(std::unique_ptr<GraphicsLogic> logic);
    GraphicsLogic* GetGraphicsLogic() { return m_graphicsLogic.get(); }
    void SetTool(std::shared_ptr<GraphicsTool> ptr);
    GraphicsTool* GetTool() const { return m_tool.get(); }
    virtual node::GraphicsObject* GetObjectAt(const model::Point& p) const;

protected:
    virtual void OnSetRect(const SDL_Rect& rect) override;
    virtual void OnMouseMove(const SDL_Point& p) override;
    virtual MI::ClickEvent OnLMBDown(const SDL_Point& p) override;
    virtual MI::ClickEvent OnLMBUp(const SDL_Point& p) override;
    virtual bool OnScroll(const double amount, const SDL_Point& p) override;

    virtual void OnDropObject(DragDropObject& object, const SDL_Point& p) override;
    virtual void OnDrawDropObject(SDL_Renderer* renderer,
        const DragDropObject& object, const SDL_Point& p) override;
    virtual void OnDropEnter(const DragDropObject& object) override;
    virtual void OnDropExit(const DragDropObject& object) override;

    virtual void OnNotify(SceneModification& e);

private:
    bool InternalSelectObject(GraphicsObject* object);

    struct DragDropDrawObject
    {
        node::model::BlockModel model;
        node::BlockStyler styler;
    };

    model::Rect m_spaceRect_base;
    model::Rect m_spaceRect;
    double m_scroll_ratio = 1.25;
    double m_zoomScale = 1;
    int m_spaceQuantization = 20;
    std::optional<SDL_Point> m_current_mouse_position;
    SDL_Point m_StartPointScreen{ 0,0 };
    SDL_Point m_startEdgeSpace{ 0,0 };
    std::vector<ObjectSlot> m_objects;
    HandlePtr<GraphicsObject> m_current_mouse_hover;
    std::vector<HandlePtr<GraphicsObject>> m_current_selection;
    std::vector<DragObject> m_drag_objects;
    std::unique_ptr<GraphicsLogic> m_graphicsLogic;
    std::shared_ptr<GraphicsTool> m_tool;
    SpaceScreenTransformer m_spaceScreenTransformer;
    std::shared_ptr<SceneModelManager> m_sceneModel;
    std::optional<DragDropDrawObject> m_dragDropDrawObject;
    CAPTURE_MODE m_mouse_capture_mode = CAPTURE_MODE::NONE;
    GraphicsSceneMode m_SceneMode = GraphicsSceneMode::Normal;
};

}