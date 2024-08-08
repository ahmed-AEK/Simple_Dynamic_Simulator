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

enum class NetOrientation
{
	Vertical,
	Horizontal
};

enum class NetSide
{
	North,
	South,
	East,
	West
};

constexpr int NET_NODE_OBJECT_Z = 100;
class GRAPHICSSCENE_API NetNode : public GraphicsObject
{
public:
	explicit NetNode(const model::Point& center, IGraphicsScene* scene = nullptr);
	virtual void Draw(SDL_Renderer* renderer) override;
	const model::Point& getCenter() noexcept { return m_centerPoint; }
	void setSegment(NetSegment* segment, NetSide side);
	NetSegment* getSegment(NetSide side)
	{
		switch (side)
		{
		case NetSide::North:
			return m_northSegment;
		case NetSide::South:
			return m_southSegment;
		case NetSide::East:
			return m_eastSegment;
		case NetSide::West:
			return m_westSegment;
		}
		return nullptr;
	}
	void setCenter(const model::Point& point) noexcept { SetSpaceOrigin({ point.x - m_width / 2, point.y - m_height / 2 }); }
	void UpdateConnectedSegments();
	void SetConnectedSocket(BlockSocketObject* socket);
	BlockSocketObject* GetConnectedSocket() noexcept;
	uint8_t GetConnectedSegmentsCount();
	void ClearSegment(const NetSegment* segment);

	void SetId(std::optional<model::NetNodeUniqueId> id) { m_id = std::move(id); }
	std::optional<model::NetNodeUniqueId> GetId() const noexcept { return m_id; }

protected:
	void OnSetSpaceRect(const model::Rect& rect) override;
private:
	std::optional<model::NetNodeUniqueId> m_id = std::nullopt;
	model::Point m_centerPoint;
	NetSegment* m_northSegment = nullptr;
	NetSegment* m_southSegment = nullptr;
	NetSegment* m_eastSegment = nullptr;
	NetSegment* m_westSegment = nullptr;
	BlockSocketObject* m_socket = nullptr;
	static constexpr int m_width = 10;
	static constexpr int m_height = 10;
};

constexpr int NET_SEGMENT_OBJECT_Z = 50;

class GRAPHICSSCENE_API NetSegment : public GraphicsObject
{
public:
	explicit NetSegment(const NetOrientation& orientation, 
	NetNode* startNode = nullptr, NetNode* endNode = nullptr, IGraphicsScene* scene = nullptr);
	virtual void Draw(SDL_Renderer* renderer) override;
	NetNode* getStartNode() noexcept { return m_startNode; }
	NetNode* getEndNode() noexcept { return m_endNode; }
	void Connect(NetNode* start, NetNode* end, const NetOrientation& orientation);
	void Disconnect();
	void CalcRect();
	const NetOrientation& GetOrientation() const noexcept { return m_orientation; }
	int GetWidth() const { return c_width; }

	void SetId(std::optional<model::NetSegmentUniqueId> id) { m_id = std::move(id); }
	std::optional<model::NetSegmentUniqueId> GetId() const noexcept { return m_id; }

protected:
private:
	static constexpr int c_width = 10;

	std::optional<model::NetSegmentUniqueId> m_id = std::nullopt;
	NetNode* m_startNode;
	NetNode* m_endNode;
	NetOrientation m_orientation{};
};

struct GRAPHICSSCENE_API NetObject
{
	std::string net_name = "net1";
	std::vector<NetNode*> m_nodes;
	std::vector<NetSegment*> m_segments;
};

};