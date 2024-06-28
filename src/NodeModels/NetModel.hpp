#pragma once

#include "NodeModels/Utils.hpp"
#include <vector>
#include <string>
#include <optional>
#include <functional>
#include <span>

namespace node::model
{

struct NetNodeModel
{
	NetNodeModel(const id_int& id, const Point& position = {})
		:m_Id{ id }, m_position{ position } {}
	id_int GetId() const noexcept { return m_Id; }
	id_int m_Id;
	Point m_position;
	std::optional<id_int> m_northSegmentId = 0;
	std::optional<id_int> m_southSegmentId = 0;
	std::optional<id_int> m_westSegmentId = 0;
	std::optional<id_int> m_eastSegmentId = 0;
};

enum class NetSegmentOrientation 
{
	horizontal,
	vertical,
};

struct NetSegmentModel
{
	const id_int GetId() const noexcept { return m_Id; }
	NetSegmentModel(id_int id, id_int first_node, id_int second_node, NetSegmentOrientation orientation)
		:m_Id{ id }, m_firstNodeId{ first_node }, m_secondNodeId{ second_node }, m_orientation{ orientation } {}
private:
	id_int m_Id;
public:
	id_int m_firstNodeId;
	id_int m_secondNodeId;
	NetSegmentOrientation m_orientation;

};

class NetModel
{
public:
	NetModel(id_int id, std::optional<std::string> name = std::nullopt)
		: m_name{ name }, m_Id{ id } {}
	id_int GetId() const noexcept { return m_Id; }
	const std::optional<std::reference_wrapper<const std::string>> GetName() const noexcept { 
		return m_name.has_value() ? 
			*m_name :
			std::optional<std::reference_wrapper<const std::string>>{};
	}
	const std::string SetName(std::string name) { m_name = name; }

	void AddNetNode(NetNodeModel netNode) { m_nodes.push_back(std::move(netNode)); }
	void RemoveNetNodeById(id_int id);

	std::optional<std::reference_wrapper<NetNodeModel>>
		GetNetNodeById(id_int id);
	std::span<NetNodeModel>
		GetNetNodes() { return m_nodes; }

	void AddNetSegment(NetSegmentModel netSegment) { m_segments.push_back(std::move(netSegment)); }
	void RemoveNetSegmentById(id_int id);

	std::optional<std::reference_wrapper<NetSegmentModel>>
		GetNetSegmentById(id_int id);
	std::span<NetSegmentModel>
		GetNetSegments() { return m_segments; }

	void ReserveNodes(size_t size) { m_nodes.reserve(size); }
	void ReserveSegments(size_t size) { m_segments.reserve(size); }
private:
	std::optional<std::string> m_name;
	id_int m_Id;
	std::vector<NetNodeModel> m_nodes;
	std::vector<NetSegmentModel> m_segments;
};

}