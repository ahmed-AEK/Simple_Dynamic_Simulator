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

private:
	NetId m_Id;
	std::optional<std::string> m_name;

};

using NetModelRef = typename std::reference_wrapper<NetModel>;
}