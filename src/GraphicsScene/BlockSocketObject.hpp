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
		model::Point center_in_block = { 0,0 }, model::ConnectedSegmentSide connection_side = {}, const model::NetCategory& category = {});

	void SetConnectedNode(NetNode* node);
	NetNode* GetConnectedNode() noexcept;

	BlockObject* GetParentBlock() const { return m_parentBlock; }

	std::optional<model::SocketUniqueId> GetUniqueId() const;
	std::optional<model::SocketId> GetId() const { return m_id; }
	void SetId(std::optional<model::SocketId> id) { m_id = std::move(id); }

	model::Point GetCenterInBlock() const
	{
		return GetPosition() + model::Point{nodeLength / 2, nodeLength / 2 };
	}
	model::Point GetCenterInSpace() const
	{
		return GetScenePosition() + model::Point{ nodeLength / 2, nodeLength / 2 };
	}
	void SetCenterInBlock(const model::Point& point) 
	{
		SetPosition({ point.x - nodeLength / 2, point.y - nodeLength / 2 });
	}
	model::BlockSocketModel::SocketType GetSocketType() const { return m_socktType; }
	model::ConnectedSegmentSide GetConnectionSide() const { return m_connection_side; }
	void SetSocketType(model::BlockSocketModel::SocketType type) { m_socktType = type; }	
	void SetConnectionSide(model::ConnectedSegmentSide side) { m_connection_side = side; }
	void SetCategory(const model::NetCategory& category) { m_category = category; }
	const model::NetCategory& GetCategory() const { return m_category; }

	void Draw(SDL::Renderer& renderer, const SpaceScreenTransformer& transformer) override;

	static constexpr int nodeLength = 15;
protected:
	void OnSetPosition(const model::Point& position) override;
private:
	void SetParentBlock(BlockObject* block) { m_parentBlock = block; }
	friend class BlockObject;

	BlockObject* m_parentBlock = nullptr;
	model::BlockSocketModel::SocketType m_socktType;
	model::ConnectedSegmentSide m_connection_side;
	std::optional<model::SocketId> m_id;
	NetNode* m_connected_node = nullptr;
	model::NetCategory m_category;
};

}