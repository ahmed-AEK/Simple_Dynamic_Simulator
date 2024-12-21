#include "BlockModel.hpp"
#include <cassert>
#include <algorithm>

node::model::BlockSocketModel* node::model::BlockModel::GetSocketById(SocketId id)
{
	auto vec = GetSockets();
	auto iter = std::find_if(vec.begin(), vec.end(),
		[id](const BlockSocketModel& socket) {return id == socket.GetId(); });
	if (iter != vec.end())
	{
		return &*iter;
	}
	return nullptr;
}

const node::model::BlockSocketModel* node::model::BlockModel::GetSocketById(SocketId id) const
{
	auto vec = GetSockets();
	auto iter = std::find_if(vec.begin(), vec.end(),
		[id](const BlockSocketModel& socket) {return id == socket.GetId(); });
	if (iter != vec.end())
	{
		return &*iter;
	}
	return nullptr;
}

void node::model::BlockModel::SetId(const BlockId& id)
{
	m_Id = id;
}

