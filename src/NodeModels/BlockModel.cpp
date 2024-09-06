#include "BlockModel.hpp"
#include <cassert>
#include <algorithm>

std::optional<std::reference_wrapper<node::model::BlockSocketModel>> 
node::model::BlockModel::GetSocketById(SocketId id)
{
	auto vec = GetSockets();
	auto iter = std::find_if(vec.begin(), vec.end(),
		[id](const BlockSocketModel& socket) {return id == socket.GetId(); });
	if (iter != vec.end())
	{
		return { *iter };
	}
	return std::nullopt;
}

std::optional<std::reference_wrapper<const node::model::BlockSocketModel>> 
node::model::BlockModel::GetSocketById(SocketId id) const
{
	auto vec = GetSockets();
	auto iter = std::find_if(vec.begin(), vec.end(),
		[id](const BlockSocketModel& socket) {return id == socket.GetId(); });
	if (iter != vec.end())
	{
		return { *iter };
	}
	return std::nullopt;
}

void node::model::BlockModel::SetId(const BlockId& id)
{
	m_Id = id;
}

