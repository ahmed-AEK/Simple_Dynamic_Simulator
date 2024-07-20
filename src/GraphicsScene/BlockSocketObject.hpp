#pragma once

#include "GraphicsScene/GraphicsObject.hpp"

namespace node
{
struct NetObject;
class NetSegment;
class NetNode;
class BlockObject;

enum class SocketType
{
	input,
	output,
	inout
};

class GRAPHICSSCENE_API BlockSocketObject: public GraphicsObject
{
public:
	void SetConnectedNode(NetNode* node);
	NetNode* GetConnectedNode() noexcept;
	static constexpr int nodeLength = 15;
	BlockSocketObject(SocketType type, IGraphicsScene* parentScene, BlockObject* parentNode);
	void OnSetSpaceRect(const model::Rect& rect) override;
	void SetPosition(SDL_Point p);
	SDL_Point GetCenter();
	void Draw(SDL_Renderer* renderer) override;
	virtual MI::ClickEvent OnLMBDown(const model::Point& current_mouse_point) override;
	SocketType GetSocketType() const { return m_socktType; }
private:
	BlockObject* m_parentNode;
	SocketType m_socktType;
	NetNode* m_connected_node = nullptr;
};

}