#pragma once

#include "GraphicsScene/GraphicsScene_exports.h"

#include "GraphicsScene/DraggableObject.hpp"

#include <memory>
#include <vector>

namespace node
{
class NodeSocket;

struct GRAPHICSSCENE_API SocketData
{
    int id;
    std::unique_ptr<NodeSocket> socket;
};

class GRAPHICSSCENE_API Node: public DraggableObject
{
public:
    Node(SDL_Rect rect, GraphicsScene* scene);
    virtual void Draw(SDL_Renderer* renderer);
    void AddInputSocket(int id);
    void AddOutputSocket(int id);
    std::vector<NodeSocket*> GetSockets();
    void DisconnectSockets();
protected:
    virtual void OnSetSpaceRect(const SDL_Rect& rect);
    virtual void PositionSockets();
    virtual GraphicsObject* OnGetInteractableAtPoint(const SDL_Point& point) override;
    virtual void OnUpdateRect();
    virtual MI::ClickEvent OnLMBDown(const SDL_Point& current_mouse_point) override;
    virtual MI::ClickEvent OnLMBUp(const SDL_Point& current_mouse_point) override;
    virtual void OnMouseMove(const SDL_Point& current_mouse_point) override;
private:
    std::vector<SocketData> m_input_sockets;
    std::vector<SocketData> m_output_sockets;
    std::vector<SocketData> m_inout_sockets;
    bool b_being_deleted;

};

};