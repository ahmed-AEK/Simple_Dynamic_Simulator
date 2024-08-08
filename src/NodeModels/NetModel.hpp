#pragma once

#include "NodeModels/Utils.hpp"
#include "NodeModels/BlockModel.hpp"
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
	enum class ConnectedSegmentSide
	{
		north=0,
		south=1,
		west=2,
		east=3,
	};

	explicit NetNodeModel(const NetNodeId& id, const Point& position = {})
		:m_Id{ id }, m_position{ position } {}
	const NetNodeId& GetId() const noexcept { return m_Id; }
	const Point& GetPosition() const noexcept { return m_position; };
	void SetPosition(const Point& position) { m_position = position; }
	std::optional<NetSegmentId> GetSegmentAt(const ConnectedSegmentSide side);
	void SetSegmentAt(const ConnectedSegmentSide side, const std::optional<NetSegmentId> segment);
private:
	NetNodeId m_Id;
	std::array<bool, 4> m_valid_sides{ false,false,false,false };
	Point m_position;
	std::array<NetSegmentId, 4> m_segmentIds{ NetSegmentId{0}, NetSegmentId{0}, NetSegmentId{0}, NetSegmentId{0}};
};

struct NetSegmentModel
{
	enum class NetSegmentOrientation
	{
		horizontal = 0,
		vertical = 1,
	};

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
	explicit NetModel(id_int id = 0, std::optional<std::string> name = std::nullopt)
		: m_name{ name }, m_Id{ id } {}

	const NetId& GetId() const noexcept { return m_Id; }
	void SetId(NetId id) { m_Id = id; }

	const std::optional<std::reference_wrapper<const std::string>> GetName() const noexcept { 
		return m_name.has_value() ? 
			*m_name :
			std::optional<std::reference_wrapper<const std::string>>{};
	}
	void SetName(std::optional<std::string> name) { m_name = std::move(name); }

	void AddNetNode(NetNodeModel&& netNode) { m_nodes.push_back(std::move(netNode)); }
	void RemoveNetNodeById(const NetNodeId& id);

	std::optional<std::reference_wrapper<NetNodeModel>>
		GetNetNodeById(const NetNodeId& id);
	std::span<NetNodeModel>
		GetNetNodes() { return m_nodes; }

	void AddNetSegment(NetSegmentModel&& netSegment) { m_segments.push_back(std::move(netSegment)); }
	void RemoveNetSegmentById(const NetSegmentId& id);

	std::optional<std::reference_wrapper<NetSegmentModel>>
		GetNetSegmentById(const NetSegmentId& id);
	std::span<NetSegmentModel>
		GetNetSegments() { return m_segments; }

	std::span<model::SocketNodeConnection> GetSocketConnections() { return m_SocketConnections; }
	void AddSocketNodeConnection(const model::SocketNodeConnection& connection);
	void RemoveSocketConnectionForSocket(const model::SocketUniqueId& socket);
	std::optional<std::reference_wrapper<node::model::SocketNodeConnection>> 
		GetSocketConnectionForNode(const model::NetNodeId& node_id);

	void ReserveNodes(size_t size) { m_nodes.reserve(size); }
	void ReserveSegments(size_t size) { m_segments.reserve(size); }
private:
	NetId m_Id;
	std::optional<std::string> m_name;
	std::vector<NetNodeModel> m_nodes;
	std::vector<NetSegmentModel> m_segments;
	std::vector<SocketNodeConnection> m_SocketConnections;
};

using NetModelRef = typename std::reference_wrapper<NetModel>;
}