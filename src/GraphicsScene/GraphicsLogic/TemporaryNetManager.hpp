#pragma once

#include "GraphicsLogic/Anchors.hpp"

namespace node
{
struct NetSolutionEndDescription;
class NetNode;
class NetSegment;
class GraphicsScene;
class Widget;

namespace logic
{
struct TemporaryNetManager
{
public:
	static TemporaryNetManager CreateFromLeafNodeNet(node::NetNode& leaf_node, GraphicsScene& scene);
	static TemporaryNetManager Create(GraphicsScene& scene);
	TemporaryNetManager(std::vector<HandlePtrS<NetNode, GraphicsObject>> orig_nodes,
		std::vector<HandlePtrS<NetSegment, GraphicsObject>> orig_segments,
		std::array<HandlePtrS<NetNode, GraphicsObject>, 6> temp_nodes,
		std::array<HandlePtrS<NetSegment, GraphicsObject>, 5> temp_segments,
		std::shared_ptr<const node::NetCategoryStyle> styler,
		GraphicsScene& scene);
	TemporaryNetManager();
	~TemporaryNetManager();
	TemporaryNetManager(TemporaryNetManager&&);
	TemporaryNetManager& operator=(TemporaryNetManager&&);

	void ResetNodes();
	void PositionNodes(const NetSolutionEndDescription& end_description);
	void CleanUp();
	void SetHighlight(bool value);
private:
	void CreateStartAnchor();
public:	
	anchor_t start_anchor;
	std::vector<HandlePtrS<NetNode, GraphicsObject>> orig_nodes;
	std::vector<HandlePtrS<NetSegment, GraphicsObject>> orig_segments;
	std::array<HandlePtrS<NetNode, GraphicsObject>, 6> temp_nodes{};
	std::array<HandlePtrS<NetSegment, GraphicsObject>, 5> temp_segments{};
	std::shared_ptr<const node::NetCategoryStyle> m_styler;
	HandlePtrS<GraphicsScene, Widget> scene;
};
}

}