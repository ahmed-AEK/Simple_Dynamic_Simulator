#pragma once

#include "NodeModels/SceneModelManager.hpp"

namespace node
{

namespace NetUtils
{
	std::optional<BlockDeletionRequest> GetDeletionRequestForBlock(model::BlockId block_id, const model::NodeSceneModel& scene);
	std::optional<NetModificationRequest> GetDeletionRequestForNet(std::span<const model::NetSegmentId> segments_to_remove,
		std::span<const model::NetNodeId> nodes_to_remove, const model::NodeSceneModel& scene);

	std::optional<model::NetCategory> GetCategoryAfterRemoval(model::NetId net_id, std::span<const model::NetSegmentId> segments_to_remove,
		std::span<const model::NetNodeId> nodes_to_remove, const model::NodeSceneModel& scene);
}

}
