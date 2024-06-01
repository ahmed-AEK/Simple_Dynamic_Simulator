#pragma once

#include "GraphicsScene/GraphicsObject.hpp"

namespace node
{
struct NetObject;
class NetSegment;
class NetNode;
class Node;

enum class SocketType
{
	input,
	output,
	inout
};

class GRAPHICSSCENE_API NodeSocket: public GraphicsObject
{
public:
	void SetConnectedNode(NetNode* node);
	NetNode* GetConnectedNode() noexcept;
	static constexpr int nodeLength = 15;
	NodeSocket(SocketType type, GraphicsScene* parentScene, Node* parentNode);
	void OnSetSpaceRect(const SDL_Rect& rect);
	void SetPosition(SDL_Point p);
	SDL_Point GetCenter();
	virtual void Draw(SDL_Renderer* renderer);
	virtual MI::ClickEvent OnLMBDown(const SDL_Point& current_mouse_point) override;
private:
	Node* m_parentNode;
	SocketType m_socktType;
	NetNode* m_connected_node = nullptr;
};

}