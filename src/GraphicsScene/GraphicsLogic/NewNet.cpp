#include "GraphicsLogic/NewNet.hpp"
#include "GraphicsScene.hpp"
#include "GraphicsScene/NetObject.hpp"
#include "GraphicsScene/BlockObject.hpp"
#include "GraphicsScene/BlockSocketObject.hpp"
#include "GraphicsScene/GraphicsObjectsManager.hpp"
#include <array>
#include <algorithm>

std::unique_ptr<node::logic::NewNetObject> node::logic::NewNetObject::Create(BlockSocketObject* socket, 
	GraphicsScene* scene, GraphicsObjectsManager* manager)
{
	assert(socket);
	assert(scene);
	std::array<NetNode*, 4> nodes{};
	std::array<NetSegment*, 3> segments{};
	try
	{
		int layer = 1000;
		for (auto&& segment : segments)
		{
			auto new_segmet = std::make_unique<NetSegment>(NetOrientation::Vertical,nullptr, nullptr,scene);
			segment = new_segmet.get();
			scene->AddObject(std::move(new_segmet), layer);
			layer++;
		}
		for (auto&& node : nodes)
		{
			auto new_node = std::make_unique<NetNode>(model::Point{ 0,0 }, scene);
			node = new_node.get();
			scene->AddObject(std::move(new_node), layer);
			layer++;
		}
		assert(manager);
		return std::make_unique<NewNetObject>(socket, nodes, segments,scene, manager);
	}
	catch (...)
	{
		// if we failed to create the object, delete everything
		for (const auto& node : nodes)
		{
			if (node)
			{
				scene->PopObject(node);
			}
		}
		for (const auto& segment : segments)
		{
			if (segment)
			{
				scene->PopObject(segment);
			}
		}
	}
	return nullptr;
}

node::logic::NewNetObject::NewNetObject(BlockSocketObject* socket, std::array<NetNode*, 4> nodes, 
	std::array<NetSegment*, 3> segments, GraphicsScene* scene, GraphicsObjectsManager* manager)
	:GraphicsLogic{scene, manager }, m_socket{socket->GetFocusHandlePtr()}
{
	
	std::transform(nodes.begin(), nodes.end(), m_nodes.begin(), 
		[](const auto& node)
		{
			assert(node);
			return node->GetFocusHandlePtr();
		});
	std::transform(segments.begin(), segments.end(), m_segments.begin(),
		[](const auto& segment)
		{
			assert(segment);
			return segment->GetFocusHandlePtr();
		});
	for (auto&& node : nodes)
	{
		node->setCenter(socket->GetCenterInSpace());
	}
	segments[0]->Connect(nodes[0], nodes[1], NetOrientation::Horizontal);
	segments[1]->Connect(nodes[1], nodes[2], NetOrientation::Vertical);
	segments[2]->Connect(nodes[2], nodes[3], NetOrientation::Horizontal);
}

static node::NetNode* AsNode(node::HandlePtr<node::GraphicsObject>& obj)
{
	assert(obj.isAlive());
	return static_cast<node::NetNode*>(obj.GetObjectPtr());
}

static node::NetSegment* AsSegment(node::HandlePtr<node::GraphicsObject>& obj)
{
	assert(obj.isAlive());
	return static_cast<node::NetSegment*>(obj.GetObjectPtr());
}

void node::logic::NewNetObject::OnMouseMove(const model::Point& current_mouse_point)
{
	BlockSocketObject* end_socket = GetSocketAt(current_mouse_point);
	
	auto end_point = current_mouse_point;
	if (end_socket)
	{
		end_point = end_socket->GetCenterInSpace();
	}

	if (!m_socket.isAlive())
	{
		GetScene()->SetGraphicsLogic(nullptr);
		return;
	}

	model::Point start = static_cast<BlockSocketObject*>(m_socket.GetObjectPtr())->GetCenterInSpace();
	model::node_int midpoint_x = (end_point.x + start.x) / 2;
	AsNode(m_nodes[1])->setCenter({ midpoint_x, start.y });
	AsNode(m_nodes[2])->setCenter({ midpoint_x, end_point.y });
	AsNode(m_nodes[3])->setCenter({ end_point.x, end_point.y });
	AsSegment(m_segments[0])->CalcRect();
	AsSegment(m_segments[1])->CalcRect();
	AsSegment(m_segments[2])->CalcRect();
}

MI::ClickEvent node::logic::NewNetObject::OnLMBUp(const model::Point& current_mouse_point)
{
	UNUSED_PARAM(current_mouse_point);
	if (!m_socket.isAlive())
	{
		DeleteAllOwnedObjects();
		return MI::ClickEvent::NONE;
	}

	assert(GetObjectsManager());
	GetObjectsManager()->GetSceneModel()->AddNewNet(PopulateResultNet(current_mouse_point));

	DeleteAllOwnedObjects();
	return MI::ClickEvent::CLICKED;
}

void node::logic::NewNetObject::OnCancel()
{
	DeleteAllOwnedObjects();
}

node::model::NetModel node::logic::NewNetObject::PopulateResultNet(const model::Point& current_mouse_point)
{
	using model::NetNodeId;
	using model::NetSegmentId;
	using enum model::NetNodeModel::ConnectedSegmentSide;

	auto* end_socket = GetSocketAt(current_mouse_point);

	node::model::NetModel net;
	std::array<NetNodeId, 4> node_ids{ NetNodeId{1},NetNodeId{2},NetNodeId{3},NetNodeId{4} };
	std::array<NetSegmentId, 3> segment_ids{ NetSegmentId{1},NetSegmentId{2},NetSegmentId{3} };

	{
		model::id_int node_id = 0;
		auto obj = model::NetNodeModel{ node_ids[node_id],
	AsNode(m_nodes[node_id])->getCenter() };
		obj.SetSegmentAt(east, segment_ids[0]);
		net.AddNetNode(std::move(obj));
	}

	{
		model::id_int node_id = 1;
		auto obj = model::NetNodeModel{ node_ids[node_id],
			AsNode(m_nodes[node_id])->getCenter() };
		obj.SetSegmentAt(west, segment_ids[0]);
		obj.SetSegmentAt(south, segment_ids[1]);
		net.AddNetNode(std::move(obj));
	}

	{
		model::id_int node_id = 2;
		auto obj = model::NetNodeModel{ model::NetNodeId{node_ids[node_id]},
	AsNode(m_nodes[node_id])->getCenter() };
		obj.SetSegmentAt(north, segment_ids[1]);
		obj.SetSegmentAt(east, segment_ids[2]);
		net.AddNetNode(std::move(obj));
	}

	{
		model::id_int node_id = 3;
		auto obj = model::NetNodeModel{ model::NetNodeId{ node_ids[node_id]},
	AsNode(m_nodes[node_id])->getCenter() };
		obj.SetSegmentAt(west, segment_ids[2]);
		net.AddNetNode(std::move(obj));
	}

	{
		model::id_int start_segment_id = 1;
		model::id_int start_node_id = 1;
		for (auto&& segment : m_segments)
		{
			auto orientation = AsSegment(segment)->GetOrientation() == NetOrientation::Horizontal ?
				model::NetSegmentModel::NetSegmentOrientation::horizontal :
				model::NetSegmentModel::NetSegmentOrientation::vertical;
			net.AddNetSegment(model::NetSegmentModel{ model::NetSegmentId{start_segment_id},
				NetNodeId{start_node_id}, NetNodeId{start_node_id + 1}, orientation });
			start_node_id++;
			start_segment_id++;
		}
	}

	auto* socket = static_cast<BlockSocketObject*>(m_socket.GetObjectPtr());

	assert(socket->GetId());
	assert(socket->GetParentBlock());
	assert(socket->GetParentBlock()->GetModelId());

	net.AddSocketNodeConnection(model::SocketNodeConnection{
		model::SocketUniqueId{*(socket->GetId()), *(socket->GetParentBlock()->GetModelId())}, node_ids[0]});
	if (end_socket)
	{
		net.AddSocketNodeConnection(model::SocketNodeConnection{
		model::SocketUniqueId{*(end_socket->GetId()), *(end_socket->GetParentBlock()->GetModelId())}, node_ids[3] });
	}
	return net;
}

node::BlockSocketObject* node::logic::NewNetObject::GetSocketAt(const model::Point& point) const
{
	const auto& blocks = GetObjectsManager()->getBlocksRegistry();

	auto current_mouse_point_SDL = ToSDLPoint(point);
	BlockSocketObject* end_socket = nullptr;
	for (const auto& block_it : blocks)
	{
		const auto& sockets = block_it.second->GetSockets();
		auto it = std::find_if(sockets.begin(), sockets.end(), [&](const auto& socket)
			{
				auto socket_rect = ToSDLRect(socket->GetSpaceRect());
				if (SDL_PointInRect(&current_mouse_point_SDL, &socket_rect))
				{
					return true;
				}
				return false;
			});
		if (it != sockets.end() && m_socket.GetObjectPtr() != *it)
		{
			end_socket = *it;
		}
	}
	return end_socket;
}

void node::logic::NewNetObject::DeleteAllOwnedObjects()
{
	GraphicsScene* scene = GetScene();
	for (auto&& node : m_nodes)
	{
		if (auto ptr = node.GetObjectPtr())
		{
			scene->PopObject(ptr);
		}
	}
	for (auto&& segment : m_segments)
	{
		if (auto ptr = segment.GetObjectPtr())
		{
			scene->PopObject(ptr);
		}
	}
}

