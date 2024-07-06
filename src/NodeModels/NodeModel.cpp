#include "NodeModel.hpp"
#include <cassert>

std::optional<std::reference_wrapper<node::model::NodeSocketModel>> 
node::model::NodeModel::GetSocketById(id_int id, const NodeSocketModel::SocketType type)
{
	auto vec = GetSockets(type);
	auto iter = std::find_if(vec.begin(), vec.end(),
		[id](const NodeSocketModel& socket) {return id == socket.GetId().m_Id; });
	if (iter != vec.end())
	{
		return { *iter };
	}
	return std::nullopt;
}

