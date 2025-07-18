#pragma once

#include "GraphicsScene/GraphicsObject.hpp"
#include "NodeModels/NetModel.hpp"
#include "GraphicsScene/NetCategoriesStyleManager.hpp"
#include <vector>
#include <variant>
#include <cassert>

namespace node {

struct NetObject;
class NetSegment;
class NetNode;
class BlockSocketObject;

void AddSelectConnectedNet(NetSegment& segment, GraphicsScene& scene);
void AddSelectConnectedNet(NetNode& node, GraphicsScene& scene);

constexpr int NET_NODE_OBJECT_Z = 100;
class GRAPHICSSCENE_API NetNode : public GraphicsObject
{
public:
	explicit NetNode(const model::Point& center);
	void Draw(SDL::Renderer& renderer, const SpaceScreenTransformer& transformer) override;
	model::Point getCenter() const { return GetPosition() + model::Point{m_width / 2, m_height / 2}; }
	void setSegment(NetSegment* segment, model::ConnectedSegmentSide side);
	NetSegment* getSegment(model::ConnectedSegmentSide side) const
	{
		assert(static_cast<size_t>(side) < 4);
		return m_connected_segments[static_cast<size_t>(side)];
	}
	std::span<NetSegment* const> getSegments() const { return m_connected_segments; }
	void setCenter(const model::Point& point) { SetPosition({ point.x - m_width / 2, point.y - m_height / 2 }); }
	void UpdateConnectedSegments();
	void SetConnectedSocket(BlockSocketObject* socket);
	BlockSocketObject* GetConnectedSocket() noexcept;
	uint8_t GetConnectedSegmentsCount() const;
	void ClearSegment(const NetSegment* segment);

	std::optional<node::model::ConnectedSegmentSide> GetSegmentSide(const NetSegment& segment) const;

	void SetId(std::optional<model::NetNodeId> id) { m_id = std::move(id); }
	std::optional<model::NetNodeId> GetId() const noexcept { return m_id; }

	void SetNetId(std::optional<model::NetId> id) { m_net_id = id; }
	std::optional<model::NetId> GetNetId() const { return m_net_id; }

protected:
private:
	std::optional<model::NetNodeId> m_id = std::nullopt;
	std::array<NetSegment*, 4> m_connected_segments{};
	std::optional<model::NetId> m_net_id;
	BlockSocketObject* m_socket = nullptr;
	static constexpr int m_width = 10;
	static constexpr int m_height = 10;
};

constexpr int NET_SEGMENT_OBJECT_Z = 50;

class GRAPHICSSCENE_API NetSegment : public GraphicsObject
{
public:
	explicit NetSegment(const model::NetSegmentOrientation& orientation,
		NetNode* startNode = nullptr, NetNode* endNode = nullptr, std::shared_ptr<const NetCategoryStyle> styler = {});
	void Draw(SDL::Renderer& renderer, const SpaceScreenTransformer& transformer) override;
	NetNode* getStartNode() const noexcept { return m_startNode; }
	NetNode* getEndNode() const noexcept { return m_endNode; }
	void Connect(NetNode* start, NetNode* end, const model::NetSegmentOrientation& orientation);
	void Disconnect();
	void CalcRect();
	const model::NetSegmentOrientation& GetOrientation() const noexcept { return m_orientation; }
	int GetWidth() const { return c_width; }

	void SetId(std::optional<model::NetSegmentId> id) { m_id = std::move(id); }
	std::optional<model::NetSegmentId> GetId() const noexcept { return m_id; }
	model::Point GetCenter() const;

	void SetStyler(std::shared_ptr<const NetCategoryStyle> styler) { m_styler = std::move(styler); }
	std::shared_ptr<const NetCategoryStyle> GetStyler() const { return m_styler; }
protected:
private:
	static constexpr int c_width = 10;

	std::optional<model::NetSegmentId> m_id = std::nullopt;
	NetNode* m_startNode;
	NetNode* m_endNode;
	model::NetSegmentOrientation m_orientation{};
	std::shared_ptr<const NetCategoryStyle> m_styler;
};

struct GRAPHICSSCENE_API NetObject
{
	std::string net_name = "net1";
	std::vector<NetNode*> m_nodes;
	std::vector<NetSegment*> m_segments;
};

};