#include "NodeModel.hpp"
#include <cassert>

void node::model::NodeModel::RemoveSocketById(id_int id)
{
	auto it = std::find_if(m_sockets.begin(), m_sockets.end(),
		[id](const NodeSocketModel& socket) {return id == socket.GetId().m_Id; });

	assert(it != m_sockets.end());

	m_sockets.erase(it);

	NodeEventArg event1{ *this, NodeEvent::SocketsChanged };
	Notify(event1);
}

std::optional<std::reference_wrapper<node::model::NodeSocketModel>> 
node::model::NodeModel::GetSocketById(id_int id)
{
	auto iter = std::find_if(m_sockets.begin(), m_sockets.end(),
		[id](const NodeSocketModel& socket) {return id == socket.GetId().m_Id; });
	if (iter != m_sockets.end())
	{
		return { *iter };
	}
	return std::nullopt;
}

