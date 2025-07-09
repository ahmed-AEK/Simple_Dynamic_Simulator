#pragma once

#include "NodeModels/Utils.hpp"
#include "NodeModels/IdTypes.hpp"

#include <vector>
#include <string>
#include <optional>
#include <span>
#include <array>

namespace node::model
{

class NetNodeModel
{
public:
	explicit NetNodeModel(const NetNodeId& id, const Point& position = {}, NetId net_id = NetId{0})
		:m_Id{ id }, m_net_id{net_id}, m_position{position} { }
	const NetNodeId& GetId() const noexcept { return m_Id; }
	const Point& GetPosition() const noexcept { return m_position; };
	void SetPosition(const Point& position) { m_position = position; }
	std::optional<NetSegmentId> GetSegmentAt(const ConnectedSegmentSide side) const;
	void SetSegmentAt(const ConnectedSegmentSide side, const std::optional<NetSegmentId> segment);
	std::optional<ConnectedSegmentSide> GetSegmentSide(NetSegmentId segment_id) const;
	void SetNetId(NetId id) { m_net_id = id; }
	NetId GetNetId() const { return m_net_id; }
private:
	NetNodeId m_Id;
	NetId m_net_id;
	std::array<bool, 4> m_valid_sides{ false,false,false,false };
	Point m_position;
	std::array<NetSegmentId, 4> m_segmentIds{ NetSegmentId{0}, NetSegmentId{0}, NetSegmentId{0}, NetSegmentId{0}};
};

struct NetSegmentModel
{
	const NetSegmentId& GetId() const noexcept { return m_Id; }
	NetSegmentModel(NetSegmentId id, NetNodeId first_node, NetNodeId second_node, NetSegmentOrientation orientation)
		:m_Id{ id }, m_firstNodeId{ first_node }, m_secondNodeId{ second_node }, m_orientation{ orientation } {}
private:
	NetSegmentId m_Id;
public:
	NetNodeId m_firstNodeId;
	NetNodeId m_secondNodeId;
	NetSegmentOrientation m_orientation;
};

struct SocketNodeConnection
{
	SocketUniqueId socketId;
	NetNodeId      NodeId;
};

class NetModel
{
public:
	explicit NetModel(NetId id, const NetCategory& category = {})
		: m_Id{ id }, m_category{ category } {}

	const NetId& GetId() const noexcept { return m_Id; }
	void SetId(NetId id) { m_Id = id; }
	
	void AddNode(NetNodeId id) { m_nodes.push_back(id); }
	void removeNode(NetNodeId id);
private:
	NetId m_Id;
	NetCategory m_category;
	std::vector<NetNodeId> m_nodes;
};

using NetModelRef = typename std::reference_wrapper<NetModel>;
}