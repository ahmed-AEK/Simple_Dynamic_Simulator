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
	explicit BlockSocketObject(model::BlockSocketModel::SocketType type, std::optional<model::SocketId> id = std::nullopt, 
		model::Point center_in_block = {0,0}, IGraphicsScene* parentScene = nullptr, BlockObject* parentBlock = nullptr);

	void SetConnectedNode(NetNode* node);
	NetNode* GetConnectedNode() noexcept;

	BlockObject* GetParentBlock() const { return m_parentBlock; }
	void SetParentBlock(BlockObject* block) { m_parentBlock = block; }

	std::optional<model::SocketId> GetId() const { return m_id; }
	void SetId(std::optional<model::SocketId> id) { m_id = std::move(id); }


	void SetCenterInBlock(const model::Point& p) { m_center_in_block = p; }
	const model::Point& GetCenterInBlock() const { return m_center_in_block; }

	model::Point GetCenterInSpace();
	void SetCenterInSpace(const model::Point& point);

	model::BlockSocketModel::SocketType GetSocketType() const { return m_socktType; }
	void SetSocketType(model::BlockSocketModel::SocketType type) { m_socktType = type; }
	void Draw(SDL_Renderer* renderer) override;

	static constexpr int nodeLength = 15;
protected:
	void OnSetSpaceRect(const model::Rect& rect) override;
private:
	model::Point m_center_in_block;
	BlockObject* m_parentBlock;
	model::BlockSocketModel::SocketType m_socktType;
	std::optional<model::SocketId> m_id;
	NetNode* m_connected_node = nullptr;
};

}