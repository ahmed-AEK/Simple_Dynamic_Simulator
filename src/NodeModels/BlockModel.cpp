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

void node::model::BlockModel::SetId(const id_int& id)
{
	m_Id = id;
	for (auto&& socket : m_input_sockets)
	{
		socket.SetId({ socket.GetId().m_Id, id });
	}
	for (auto&& socket : m_output_sockets)
	{
		socket.SetId({ socket.GetId().m_Id, id });
	}
}

