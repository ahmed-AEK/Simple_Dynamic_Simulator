#pragma once

#include "GraphicsScene/GraphicsScene_exports.h"

#include "GraphicsScene/GraphicsObject.hpp"

#include <memory>
#include <vector>
#include "NodeModels/BlockModel.hpp"
namespace node
{
class BlockSocketObject;

namespace model
{
    class BlockModel;
}
class BlockStyler;
class BlockResizeObject;

class GRAPHICSSCENE_API BlockObject: public GraphicsObject
{
public:
    static std::unique_ptr<BlockObject> Create(GraphicsScene* scene, const model::BlockModel& model,
        std::unique_ptr<BlockStyler> styler = nullptr);

    explicit BlockObject(GraphicsScene* scene = nullptr, const model::Rect& rect = {100,100,100,100}, 
        std::unique_ptr<BlockStyler> styler = nullptr, std::optional<model::BlockId> model_id = std::nullopt, 
        model::BlockOrientation orientation = model::BlockOrientation::LeftToRight);
    ~BlockObject() override;
    void Draw(SDL_Renderer* renderer, const SpaceScreenTransformer& transformer) override;

    std::optional<model::BlockId> GetModelId();
    const std::vector<std::unique_ptr<BlockSocketObject>>& GetSockets() const;
    std::optional<std::reference_wrapper<node::BlockSocketObject>> GetSocketById(model::SocketId id);

    void UpdateStyler(const model::BlockModel& model);
    const BlockStyler& GetStyler() const { return *m_styler; }
    void RenewSockets(std::span<const model::BlockSocketModel> new_sockets);
    void SetResizeHandles(BlockResizeObject& resize_object);
    void HideResizeHandles();

    void SetOrientation(model::BlockOrientation orientation) { m_orientation = orientation; }
    model::BlockOrientation GetOrienation() const { return m_orientation; }

protected:
    void AddSocket(std::unique_ptr<BlockSocketObject> id);
    void OnSetSpaceRect(const model::Rect& rect) override;
    virtual void RePositionSockets();
    GraphicsObject* OnGetInteractableAtPoint(const model::Point& point) override;
    void OnUpdateRect() override;
    MI::ClickEvent OnLMBDown(const model::Point& current_mouse_point) override;

    void OnSceneChange() override;


private:
    std::vector<std::unique_ptr<BlockSocketObject>> m_sockets;
    std::optional<model::BlockId> m_id;
    std::unique_ptr<BlockStyler> m_styler;
    HandlePtr<GraphicsObject> m_resizer;
    model::BlockOrientation m_orientation;
};

};