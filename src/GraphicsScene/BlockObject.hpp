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
    struct BlockDataCRef;
}
class BlockStyler;
class ObjectAttachment;

class GRAPHICSSCENE_API BlockObject: public GraphicsObject
{
public:
    static std::unique_ptr<BlockObject> Create(const model::BlockModel& model,
        std::unique_ptr<BlockStyler> styler = nullptr);

    explicit BlockObject(const model::ObjectSize& size = {100,100},
        std::unique_ptr<BlockStyler> styler = nullptr, std::optional<model::BlockId> model_id = std::nullopt, 
        model::BlockOrientation orientation = model::BlockOrientation::LeftToRight);
    ~BlockObject() override;
    void Draw(SDL::Renderer& renderer, const SpaceScreenTransformer& transformer) override;

    std::optional<model::BlockId> GetModelId() { return m_id; }
    const std::vector<std::unique_ptr<BlockSocketObject>>& GetSockets() const;
    node::BlockSocketObject* GetSocketById(model::SocketId id);

    void UpdateStyler(const model::BlockDataCRef& model);
    const BlockStyler& GetStyler() const { return *m_styler; }
    void RenewSockets(std::span<const model::BlockSocketModel> new_sockets);

    void SetOrientation(model::BlockOrientation orientation) { m_orientation = orientation; }
    model::BlockOrientation GetOrienation() const { return m_orientation; }

protected:
    void AddSocket(std::unique_ptr<BlockSocketObject> id);
    GraphicsObject* OnGetInteractableAtPoint(const model::Point& point) override;
    MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;
    void OnSetPosition(const model::Point& position) override;
    void OnSetSize(const model::ObjectSize& size) override;
private:
    std::vector<std::unique_ptr<BlockSocketObject>> m_sockets;
    std::optional<model::BlockId> m_id;
    std::unique_ptr<BlockStyler> m_styler;
    model::BlockOrientation m_orientation;
};

};