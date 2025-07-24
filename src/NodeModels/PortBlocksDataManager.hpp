#pragma once

#include "NodeModels/FunctionalBlocksDataManager.hpp"
#include "NodeModels/BlockModel.hpp"

namespace node
{
namespace model
{

struct PortBlockData
{
	SocketId id{ 0 };
	SocketType port_type{};
	NetCategory category;
};

namespace detail
{
	using PortBlocksDataManagerBase = BlocksDataManager<PortBlockData>;
}

class PortBlocksDataManager : public detail::PortBlocksDataManagerBase
{
public:
	void AddPortForId(BlockId id, SocketType socket_type)
	{
		SocketId new_id{ static_cast<model::id_int>(GetData().size()) };
		SetDataForId(id, { new_id, socket_type, {} });
	}

	bool RemovePortForId(BlockId id);
};

}
}