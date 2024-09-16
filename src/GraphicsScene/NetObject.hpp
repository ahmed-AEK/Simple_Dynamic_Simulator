#pragma once

#include "GraphicsScene/GraphicsObject.hpp"
#include "NodeModels/NetModel.hpp"
#include <vector>
#include <variant>

namespace node {

struct NetObject;
class NetSegment;
class NetNode;
class IGraphicsScene;
class BlockSocketObject;


constexpr int NET_NODE_OBJECT_Z = 100;
class GRAPHICSSCENE_API NetNode : public GraphicsObject
{
public:
	explicit NetNode(const model::Point& center, IGraphicsScene* scene = nullptr);
	virtual void Draw(SDL_Renderer* renderer) override;
	const model::Point& getCenter() noexcept { return m_centerPoint; }
	void setSegment(NetSegment* segment, model::ConnectedSegmentSide side);
	NetSegment* getSegment(model::ConnectedSegmentSide side)
	{
		assert(static_cast<size_t>(side) < 4);
		return m_connected_segments[static_cast<size_t>(side)];
	}
	void setCenter(const model::Point& point) noexcept { SetSpaceOrigin({ point.x - m_width / 2, point.y - m_height / 2 }); }
	void UpdateConnectedSegments();
	void SetConnectedSocket(BlockSocketObject* socket);
	BlockSocketObject* GetConnectedSocket() noexcept;
	uint8_t GetConnectedSegmentsCount();
	void ClearSegment(const NetSegment* segment);

	std::optional<node::model::ConnectedSegmentSide> GetSegmentSide(const NetSegment& segment) const;

	void SetId(std::optional<model::NetNodeId> id) { m_id = std::move(id); }
	std::optional<model::NetNodeId> GetId() const noexcept { return m_id; }

protected:
	void OnSetSpaceRect(const model::Rect& rect) override;
private:
	std::optional<model::NetNodeId> m_id = std::nullopt;
	model::Point m_centerPoint;
	std::array<NetSegment*, 4> m_connected_segments{};
	BlockSocketObject* m_socket = nullptr;
	static constexpr int m_width = 10;
	static constexpr int m_height = 10;
};

constexpr int NET_SEGMENT_OBJECT_Z = 50;

class GRAPHICSSCENE_API NetSegment : public GraphicsObject
{
public:
	explicit NetSegment(const model::NetSegmentOrientation& orientation,
	NetNode* startNode = nullptr, NetNode* endNode = nullptr, IGraphicsScene* scene = nullptr);
	virtual void Draw(SDL_Renderer* renderer) override;
	NetNode* getStartNode() noexcept { return m_startNode; }
	NetNode* getEndNode() noexcept { return m_endNode; }
	void Connect(NetNode* start, NetNode* end, const model::NetSegmentOrientation& orientation);
	void Disconnect();
	void CalcRect();
	const model::NetSegmentOrientation& GetOrientation() const noexcept { return m_orientation; }
	int GetWidth() const { return c_width; }

	void SetId(std::optional<model::NetSegmentId> id) { m_id = std::move(id); }
	std::optional<model::NetSegmentId> GetId() const noexcept { return m_id; }

protected:
private:
	static constexpr int c_width = 10;

	std::optional<model::NetSegmentId> m_id = std::nullopt;
	NetNode* m_startNode;
	NetNode* m_endNode;
	model::NetSegmentOrientation m_orientation{};
};

struct GRAPHICSSCENE_API NetObject
{
	std::string net_name = "net1";
	std::vector<NetNode*> m_nodes;
	std::vector<NetSegment*> m_segments;
};

};