#pragma once

#include "GraphicsScene/GraphicsObject.hpp"
#include "NodeModels/BlockModel.hpp"

namespace node
{
struct NetObject;
class NetSegment;
class NetNode;
class BlockObject;

class GRAPHICSSCENE_API BlockSocketObject: public GraphicsObject
{
public:	
	BlockSocketObject(model::BlockSocketModel::SocketId id, model::BlockSocketModel::SocketType type, IGraphicsScene* parentScene, BlockObject* parentNode);

	void SetConnectedNode(NetNode* node);
	NetNode* GetConnectedNode() noexcept;
	static constexpr int nodeLength = 15;
	void OnSetSpaceRect(const model::Rect& rect) override;
	void SetPosition(SDL_Point p);
	model::Point GetCenter();
	void Draw(SDL_Renderer* renderer) override;
	virtual MI::ClickEvent OnLMBDown(const model::Point& current_mouse_point) override;
	model::BlockSocketModel::SocketType GetSocketType() const { return m_socktType; }
private:
	BlockObject* m_parentNode;
	model::BlockSocketModel::SocketType m_socktType;
	model::BlockSocketModel::SocketId m_id;
	NetNode* m_connected_node = nullptr;
};

}