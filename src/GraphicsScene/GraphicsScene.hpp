#pragma once

#include "GraphicsScene/GraphicsScene_exports.h"
#include "toolgui/Widget.hpp"
#include "NodeSDLStylers/SpaceScreenTransformer.hpp"
#include <vector>
#include <span>
#include "GraphicsScene/tools/GraphicsTool.hpp"
#include <optional>
#include "NodeModels/Observer.hpp"

namespace node
{

namespace logic
{
    class GraphicsLogic;

}

class BlockObject;
class BlockSocketObject;
class GraphicsObject;
class Scene;
class BlockStyler;

struct GRAPHICSSCENE_API ObjectSlot
{
    std::unique_ptr<node::GraphicsObject> m_ptr;
    int z_order;
};

struct GRAPHICSSCENE_API DragObject
{
    HandlePtr<GraphicsObject> m_object;
    SDL_Point m_start_position;
};
struct BlockObjectDropped
{
    DragDropObject& object;
    const SDL_Point& p;
};

class GRAPHICSSCENE_API GraphicsScene: public node::Widget, public node::SinglePublisher<BlockObjectDropped>
{
public:
    enum class CAPTURE_MODE
    {
        NONE,
        OBJECT,
        OBJECTS_DRAG,
        SCREEN_DRAG
    };

    static constexpr int BlockLayer = 100;
    static constexpr int SegmentLayer = 200;
    static constexpr int NetNodeLayer = 300;
    static constexpr int InteractiveLayer = 400;

    GraphicsScene(const SDL_Rect& rect, node::Scene* parent);
    ~GraphicsScene() override;

    void SetScrollRatio(double scroll_ratio) { m_scroll_ratio = scroll_ratio; }
    double GetScrollRatio() const noexcept { return m_scroll_ratio; }
    double GetZoomScale() const { return m_zoomScale; }

    void AddObject(std::unique_ptr<node::GraphicsObject> obj, int z_order);
    std::unique_ptr<node::GraphicsObject> PopObject(const node::GraphicsObject* obj);
    void ClearAllObjects();

    void UpdateObjectsRect();

    void SetSpaceRect(const model::Rect& rect);
    const model::Rect& GetSpaceRect() const noexcept;

    std::span<const HandlePtr<GraphicsObject>> GetCurrentSelection() const;
    std::span<HandlePtr<GraphicsObject>> GetCurrentSelection();
    void AddSelection(HandlePtr<GraphicsObject> handle);
    bool IsObjectSelected(const GraphicsObject& obj) const;
    void ClearCurrentSelection();

    const SpaceScreenTransformer& GetSpaceScreenTransformer() const;
    model::Point QuantizePoint(const model::Point& p);

    void Draw(SDL_Renderer* renderer) override;

    GraphicsObject* GetCurrentHover() noexcept { return m_current_mouse_hover.GetObjectPtr(); }
    void SetCurrentHover(GraphicsObject* current_hover);
    bool IsMouseCaptured() const { return m_mouse_capture_mode != CAPTURE_MODE::NONE; }
    BlockSocketObject* GetSocketAt(const model::Point space_point);

    void SetGraphicsLogic(std::unique_ptr<logic::GraphicsLogic> logic);
    logic::GraphicsLogic* GetGraphicsLogic() { return m_graphicsLogic.get(); }
    void CancelCurrentLogic();

    void SetTool(std::shared_ptr<GraphicsTool> ptr);
    GraphicsTool* GetTool() const { return m_tool.get(); }
    virtual node::GraphicsObject* GetObjectAt(const model::Point& p) const;
protected:
    void OnSetRect(const SDL_Rect& rect) override;
    void OnMouseMove(const SDL_Point& p) override;
    MI::ClickEvent OnLMBDown(const SDL_Point& p) override;
    MI::ClickEvent OnLMBUp(const SDL_Point& p) override;
    bool OnScroll(const double amount, const SDL_Point& p) override;

    void OnDropObject(DragDropObject& object, const SDL_Point& p) override;
    void OnDrawDropObject(SDL_Renderer* renderer,
        const DragDropObject& object, const SDL_Point& p) override;
    void OnDropEnter(const DragDropObject& object) override;
    void OnDropExit(const DragDropObject& object) override;


private:

    struct DragDropDrawObject
    {
        node::model::BlockModel model;
        std::shared_ptr<node::BlockStyler> styler;
    };

    model::Rect m_spaceRect_base;
    model::Rect m_spaceRect;
    double m_scroll_ratio = 1.25;
    double m_zoomScale = 1;
    int m_spaceQuantization = 20;

    SDL_Point m_current_mouse_position{ 0,0 };
    
    std::vector<ObjectSlot> m_objects;
    HandlePtr<GraphicsObject> m_current_mouse_hover;
    std::vector<HandlePtr<GraphicsObject>> m_current_selection;
    
    std::unique_ptr<logic::GraphicsLogic> m_graphicsLogic;
    std::shared_ptr<GraphicsTool> m_tool;
    
    SpaceScreenTransformer m_spaceScreenTransformer;

    std::optional<DragDropDrawObject> m_dragDropDrawObject;
    CAPTURE_MODE m_mouse_capture_mode = CAPTURE_MODE::NONE;
};

}