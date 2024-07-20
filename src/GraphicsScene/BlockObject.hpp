#pragma once

#include "GraphicsScene/GraphicsScene_exports.h"

#include "GraphicsScene/DraggableObject.hpp"

#include <memory>
#include <vector>

namespace node
{
class BlockSocketObject;

struct GRAPHICSSCENE_API SocketData
{
    int id;
    std::unique_ptr<BlockSocketObject> socket;
};

class GRAPHICSSCENE_API BlockObject: public DraggableObject
{
public:
    BlockObject(model::Rect rect, IGraphicsScene* scene);
    void Draw(SDL_Renderer* renderer) override;
    void AddInputSocket(int id);
    void AddOutputSocket(int id);
    std::vector<BlockSocketObject*> GetSockets();
    void DisconnectSockets();
protected:
    void OnSetSpaceRect(const model::Rect& rect) override;
    virtual void PositionSockets();
    GraphicsObject* OnGetInteractableAtPoint(const model::Point& point) override;
    void OnUpdateRect() override;
    MI::ClickEvent OnLMBDown(const model::Point& current_mouse_point) override;
private:
    std::vector<SocketData> m_input_sockets;
    std::vector<SocketData> m_output_sockets;
    bool b_being_deleted = false;

};

};