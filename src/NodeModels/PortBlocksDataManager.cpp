#include "PortBlocksDataManager.hpp"

bool node::model::PortBlocksDataManager::RemovePortForId(BlockId id)
{
	
	auto* block_data_ptr = GetDataForId(id);
	auto erased_socket_id = block_data_ptr->id;
	auto result = EraseDataForId(id);
	if (!result)
	{
		return false;
	}
	if (GetData().size() == 0)
	{
		// we erased the last one anyway
		return true;
	}
	struct BlockandSocketIdStore
	{
		BlockId block_id;
		PortBlockData data;
	};

	std::vector<BlockandSocketIdStore> sockets;

	for (auto&& [block_id, block_data] : GetData())
	{
		if (block_data.id.value > erased_socket_id.value)
		{
			sockets.push_back({ block_id, block_data });
		}
	}
	if (sockets.size() == 0)
	{
		return true;
	}
	std::sort(sockets.begin(), sockets.end(), [](const BlockandSocketIdStore& lhs, const BlockandSocketIdStore& rhs) { return lhs.data.id.value > rhs.data.id.value; });
	SetDataForId(sockets[0].block_id, { erased_socket_id, sockets[0].data.port_type });
	return true;
}
