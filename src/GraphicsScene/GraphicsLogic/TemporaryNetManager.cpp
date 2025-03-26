#include "TemporaryNetManager.hpp"
#include "GraphicsScene/GraphicsScene.hpp"
#include "GraphicsScene/SolverUtils.hpp"
#include "GraphicsScene/BlockSocketObject.hpp"
#include "GraphicsScene/NetObject.hpp"

node::logic::TemporaryNetManager node::logic::TemporaryNetManager::CreateFromLeafNodeNet(
	node::NetNode& leaf_node, GraphicsScene& scene)
{
	assert(leaf_node.GetConnectedSegmentsCount() == 1);

	auto branch = GetNetBranchForLeafNode(leaf_node);

	std::reverse(branch.nodes.begin(), branch.nodes.end());
	std::reverse(branch.segments.begin(), branch.segments.end());

	std::array<HandlePtrS<NetNode, GraphicsObject>, 6> temp_nodes{};
	std::array<HandlePtrS<NetSegment, GraphicsObject>, 5> temp_segments{};
	try
	{
		for (auto&& segment : temp_segments)
		{
			auto new_segmet = std::make_unique<NetSegment>(model::NetSegmentOrientation::vertical, nullptr, nullptr);
			segment.reset(*new_segmet);
			scene.AddObject(std::move(new_segmet), GraphicsScene::SegmentLayer);
		}
		for (auto&& node : temp_nodes)
		{
			auto new_node = std::make_unique<NetNode>(model::Point{ 0,0 });
			node.reset(*new_node);
			scene.AddObject(std::move(new_node), GraphicsScene::NetNodeLayer);
		}
	}
	catch (...)
	{
		for (auto& node : branch.nodes)
		{
			node->SetVisible(true);
		}
		for (auto& segment : branch.segments)
		{
			segment->SetVisible(true);
		}
		// if we failed to create the object, delete everything
		for (auto& node : temp_nodes)
		{
			if (node)
			{
				scene.PopObject(node.GetObjectPtr());
			}
		}
		for (auto& segment : temp_segments)
		{
			if (segment)
			{
				scene.PopObject(segment.GetObjectPtr());
			}
		}
		throw;
	}
	return TemporaryNetManager{std::move(branch.nodes), std::move(branch.segments), 
		std::move(temp_nodes), std::move(temp_segments),
		scene
	};
}

node::logic::TemporaryNetManager node::logic::TemporaryNetManager::Create(GraphicsScene& scene)
{
	
	std::array<HandlePtrS<NetNode, GraphicsObject>, 6> temp_nodes{};
	std::array<HandlePtrS<NetSegment, GraphicsObject>, 5> temp_segments{};
	try
	{
		for (auto&& segment : temp_segments)
		{
			auto new_segmet = std::make_unique<NetSegment>(model::NetSegmentOrientation::vertical, nullptr, nullptr);
			segment.reset(*new_segmet);
			scene.AddObject(std::move(new_segmet), GraphicsScene::SegmentLayer);
		}
		for (auto&& node : temp_nodes)
		{
			auto new_node = std::make_unique<NetNode>(model::Point{ 0,0 });
			node.reset(*new_node);
			scene.AddObject(std::move(new_node), GraphicsScene::NetNodeLayer);
		}
	}
	catch (...)
	{
		// if we failed to create the object, delete everything
		for (auto& node : temp_nodes)
		{
			if (node)
			{
				scene.PopObject(node.GetObjectPtr());
			}
		}
		for (auto& segment : temp_segments)
		{
			if (segment)
			{
				scene.PopObject(segment.GetObjectPtr());
			}
		}
		throw;
	}
	return TemporaryNetManager{ {}, {},
		std::move(temp_nodes), std::move(temp_segments),
		scene
	};
}

node::logic::TemporaryNetManager::TemporaryNetManager(
	std::vector<HandlePtrS<NetNode, GraphicsObject>> orig_nodes_, 
	std::vector<HandlePtrS<NetSegment, GraphicsObject>> orig_segments_, 
	std::array<HandlePtrS<NetNode, GraphicsObject>, 6> temp_nodes_, 
	std::array<HandlePtrS<NetSegment, GraphicsObject>, 5> temp_segments_,
	GraphicsScene& scene_
)
	:orig_nodes{std::move(orig_nodes_)}, orig_segments{std::move(orig_segments_)},
	temp_nodes{std::move(temp_nodes_)}, temp_segments{std::move(temp_segments_)},
	scene{scene_}
{
	CreateStartAnchor();

	for (auto& node : orig_nodes)
	{
		node->SetVisible(false);
	}
	for (auto& segment : orig_segments)
	{
		segment->SetVisible(false);
	}
	for (auto& node : temp_nodes)
	{
		node->SetSelected(true);
	}
	for (auto& segment : temp_segments)
	{
		segment->SetSelected(true);
	}
	ResetNodes();
}

node::logic::TemporaryNetManager::TemporaryNetManager() = default;

node::logic::TemporaryNetManager::~TemporaryNetManager()
{
	CleanUp();
}

node::logic::TemporaryNetManager::TemporaryNetManager(TemporaryNetManager&&) = default;

node::logic::TemporaryNetManager& node::logic::TemporaryNetManager::operator=(TemporaryNetManager&&) = default;


void node::logic::TemporaryNetManager::ResetNodes()
{
	for (auto& node : temp_nodes)
	{
		node->SetVisible(false);
	}
	for (auto& segment : temp_segments)
	{
		segment->SetVisible(false);
		segment->Disconnect();
	}
}

void node::logic::TemporaryNetManager::PositionNodes(const NetSolutionEndDescription& end_description)
{
	ResetNodes();

	model::Point start = std::visit(AnchorStart{}, start_anchor);

	if (std::abs(start.x - end_description.point.x) < 5 && std::abs(start.y - end_description.point.y) < 5)
	{
		temp_nodes[0]->SetVisible(true);
		temp_nodes[0]->setCenter({ start.x, start.y });
	}
	else
	{
		NetsSolver solver;
		solver.SetStartDescription(NetSolutionEndDescription{ start, std::visit(AnchorGetConnectionSide{}, start_anchor) });
		solver.SetEndDescription(end_description);
		auto solution = solver.Solve();
		ApplySolutionToNodes(solution, temp_nodes, temp_segments);
	}
}

void node::logic::TemporaryNetManager::CleanUp()
{
	auto* scene_ptr = scene.GetObjectPtr();
	if (!scene_ptr)
	{
		return;
	}

	for (auto& node : orig_nodes)
	{
		if (node)
		{
			node->SetVisible(true);
		}
	}
	for (auto& segment : orig_segments)
	{
		if (segment)
		{
			segment->SetVisible(true);
		}
	}

	for (auto& node : temp_nodes)
	{
		if (auto obj = node.GetObjectPtr())
		{
			scene_ptr->PopObject(obj);
			node = nullptr;
		}
	}
	for (auto& segment : temp_segments)
	{
		if (auto obj = segment.GetObjectPtr())
		{
			scene_ptr->PopObject(obj);
			segment = nullptr;
		}
	}
	scene = nullptr;
}

void node::logic::TemporaryNetManager::SetHighlight(bool value)
{
	for (auto& node : temp_nodes)
	{
		node->SetSelected(value);
	}
	for (auto& segment : temp_segments)
	{
		segment->SetSelected(value);
	}
}


void node::logic::TemporaryNetManager::CreateStartAnchor()
{
	start_anchor = logic::CreateStartAnchor(orig_nodes, orig_segments);
}

