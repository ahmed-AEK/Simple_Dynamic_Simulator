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
    const SDL_FPoint& p;
};

class ToolHandler
{
public:
    virtual bool IsCapturingMouse() const = 0;
    virtual void OnMouseMove(GraphicsTool::MouseHoverEvent& e) = 0;
    virtual MI::ClickEvent OnLMBDown(GraphicsTool::MouseButtonEvent& e) = 0;
    virtual MI::ClickEvent OnLMBUp(GraphicsTool::MouseButtonEvent& e) = 0;
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

    GraphicsScene(const WidgetSize& size, node::Widget* parent);
    ~GraphicsScene() override;

    float GetScrollRatio() const noexcept { return m_scroll_ratio; }

    void AddObject(std::unique_ptr<node::GraphicsObject> obj, int z_order);
    std::unique_ptr<node::GraphicsObject> PopObject(const node::GraphicsObject* obj);
    void ClearAllObjects();

    void BumpObjectInLayer(node::GraphicsObject* obj);

    void SetSpaceRect(const model::Rect& rect);
    const model::Rect& GetSpaceRect() const noexcept;

    std::span<const HandlePtr<GraphicsObject>> GetCurrentSelection() const;
    std::span<HandlePtr<GraphicsObject>> GetCurrentSelection();
    void AddSelection(HandlePtr<GraphicsObject> handle);
    void SetSelection(std::vector<GraphicsObject*> objects);
    bool IsObjectSelected(const GraphicsObject& obj) const;
    void ClearCurrentSelection();

    const SpaceScreenTransformer& GetSpaceScreenTransformer() const;
    model::Point QuantizePoint(const model::Point& p);

    GraphicsObject* GetCurrentHover() noexcept { return m_current_mouse_hover.GetObjectPtr(); }
    void SetCurrentHover(GraphicsObject* current_hover, MI::MouseHoverEvent<GraphicsObject>& e);
    bool IsMouseCaptured() const { return m_mouse_capture_mode != CAPTURE_MODE::NONE; }
    BlockSocketObject* GetSocketAt(const model::Point space_point);

    void SetGraphicsLogic(std::unique_ptr<logic::GraphicsLogic> logic);
    logic::GraphicsLogic* GetGraphicsLogic() { return m_graphicsLogic.get(); }
    void CancelCurrentLogic();

    void SetTool(std::shared_ptr<ToolHandler> ptr);
    const std::shared_ptr<ToolHandler>& GetToolHandleer() const { return m_tool; }
    virtual node::GraphicsObject* GetObjectAt(const model::Point& p) const;
    void SetZoomFactor(float value);
    float GetZoomFactor() const { return m_space_to_screen_factor; }
protected:
    void OnSetSize(const WidgetSize& size) override;
    void OnMouseMove(MouseHoverEvent& e) override;
    MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;
    MI::ClickEvent OnLMBUp(MouseButtonEvent& e) override;
    bool OnScroll(const double amount, const SDL_FPoint& p) override;

    void OnDropObject(DragDropObject& object, const SDL_FPoint& p) override;
    void OnDrawDropObject(SDL_Renderer* renderer,
        const DragDropObject& object, const SDL_FPoint& p) override;
    void OnDropEnter(const DragDropObject& object) override;
    void OnDropExit(const DragDropObject& object) override;

    void OnDraw(SDL::Renderer& renderer) override;
    virtual void OnDrawObjects(SDL::Renderer& renderer);

private:

    struct DragDropDrawObject
    {
        node::model::BlockModel model;
        std::shared_ptr<node::BlockStyler> styler;
    };

    static constexpr int m_spaceQuantization = 20;
    static constexpr float min_zoom_factor = 0.5;
    static constexpr float max_zoom_factor = 2;
    float m_space_to_screen_factor = 1;
    static constexpr float m_scroll_ratio = 1.25;

    model::Rect m_spaceRect;

    SDL_FPoint m_current_mouse_position{ 0,0 };
    
    std::vector<ObjectSlot> m_objects;
    HandlePtr<GraphicsObject> m_current_mouse_hover;
    std::vector<HandlePtr<GraphicsObject>> m_current_selection;
    
    std::unique_ptr<logic::GraphicsLogic> m_graphicsLogic;
    std::shared_ptr<ToolHandler> m_tool;
    
    SpaceScreenTransformer m_spaceScreenTransformer;

    std::optional<DragDropDrawObject> m_dragDropDrawObject;
    CAPTURE_MODE m_mouse_capture_mode = CAPTURE_MODE::NONE;
};

}