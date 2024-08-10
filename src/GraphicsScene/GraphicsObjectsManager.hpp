#pragma once

#include "NodeModels/SceneModelManager.hpp"
#include "GraphicsScene/GraphicsScene.hpp"
#include <unordered_map>

namespace node
{

class BlockObject;
class NetSegment;
class NetNode;

class GraphicsObjectsManager: public node::SingleObserver<SceneModification>, public node::SingleObserver<BlockObjectDropped>
{
public:
	GraphicsObjectsManager(GraphicsScene& scene);
	
	GraphicsScene* GetScene() { return m_scene; }

	void SetSceneModel(std::shared_ptr<SceneModelManager> scene);
	std::shared_ptr<SceneModelManager> GetSceneModel() { return m_sceneModel; }
	
	const std::unordered_map<model::BlockId, BlockObject*>& getBlocksRegistry() const { return m_blocks; }
protected:
	void OnNotify(SceneModification& e) override;
	void OnNotify(BlockObjectDropped& object) override;
private:
	void HandleNetUpdate(NetModificationReport& report);
	GraphicsScene* m_scene;
	std::shared_ptr<SceneModelManager> m_sceneModel;
	std::unordered_map<model::BlockId, BlockObject*> m_blocks;
	std::unordered_map<model::NetSegmentId, NetSegment*> m_net_segments;
	std::unordered_map<model::NetNodeId, NetNode*> m_net_nodes;

};

}