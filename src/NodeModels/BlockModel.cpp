#include "BlockModel.hpp"
#include <cassert>

std::optional<std::reference_wrapper<node::model::BlockSocketModel>> 
node::model::BlockModel::GetSocketById(id_int id, const BlockSocketModel::SocketType type)
{
	auto vec = GetSockets(type);
	auto iter = std::find_if(vec.begin(), vec.end(),
		[id](const BlockSocketModel& socket) {return id == socket.GetId().m_Id; });
	if (iter != vec.end())
	{
		return { *iter };
	}
	return std::nullopt;
}

