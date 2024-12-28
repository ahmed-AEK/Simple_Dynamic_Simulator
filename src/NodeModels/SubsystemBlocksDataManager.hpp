#pragma once

#include "NodeModels/IdTypes.hpp"
#include "NodeModels/FunctionalBlocksDataManager.hpp"

#include <string>
#include <unordered_map>

namespace node
{
namespace model
{

struct SubsystemBlockData
{
	std::string URL;
	SubSceneId scene_id;
};

using SubsystemBlocksDataManager = BlocksDataManager<SubsystemBlockData>;

}
}