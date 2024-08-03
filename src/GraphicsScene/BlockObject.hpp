#pragma once

#include "GraphicsScene/GraphicsScene_exports.h"

#include "GraphicsScene/DraggableObject.hpp"

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
    explicit BlockObject(IGraphicsScene* scene, std::shared_ptr<model::BlockModel> model = nullptr, std::shared_ptr<BlockStyler> styler = nullptr);
    ~BlockObject() override;
    void Draw(SDL_Renderer* renderer) override;

    model::BlockId GetModelId();
    std::vector<BlockSocketObject*> GetSockets();
protected:
    void AddInputSocket(model::SocketId id);
    void AddOutputSocket(model::SocketId id);
    void OnSetSpaceRect(const model::Rect& rect) override;
    virtual void RePositionSockets();
    GraphicsObject* OnGetInteractableAtPoint(const model::Point& point) override;
    void OnUpdateRect() override;
    MI::ClickEvent OnLMBDown(const model::Point& current_mouse_point) override;
private:
    std::vector<std::unique_ptr<BlockSocketObject>> m_sockets;
    std::shared_ptr<model::BlockModel> m_model;
    std::shared_ptr<BlockStyler> m_styler;
    bool b_being_deleted = false;

};

};