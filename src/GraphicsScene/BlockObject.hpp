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

class GRAPHICSSCENE_API BlockObject: public GraphicsObject
{
public:
    static std::unique_ptr<BlockObject> Create(IGraphicsScene* scene, const model::BlockModel& model,
        std::unique_ptr<BlockStyler> styler = nullptr);

    explicit BlockObject(IGraphicsScene* scene, const model::Rect& rect = {100,100,100,100}, 
        std::unique_ptr<BlockStyler> styler = nullptr, std::optional<model::BlockId> model_id = std::nullopt);
    ~BlockObject() override;
    void Draw(SDL_Renderer* renderer) override;

    std::optional<model::BlockId> GetModelId();
    const std::vector<std::unique_ptr<BlockSocketObject>>& GetSockets() const;
protected:
    void AddSocket(std::unique_ptr<BlockSocketObject> id);
    void OnSetSpaceRect(const model::Rect& rect) override;
    virtual void RePositionSockets();
    GraphicsObject* OnGetInteractableAtPoint(const model::Point& point) override;
    void OnUpdateRect() override;
    MI::ClickEvent OnLMBDown(const model::Point& current_mouse_point) override;
private:
    std::vector<std::unique_ptr<BlockSocketObject>> m_sockets;
    std::optional<model::BlockId> m_id;
    std::unique_ptr<BlockStyler> m_styler;
};

};