#pragma once

#include "NodeModels/Observer.hpp"
#include "NodeModels/IdTypes.hpp"

namespace node
{
namespace model
{
struct BlockPortsUpdate
{
	SubSceneId scene_id;
	BlockId block_id;
};

}
}