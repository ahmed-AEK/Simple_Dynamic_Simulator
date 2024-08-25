#include "SimulatorRunner.hpp"
#include "NodeModels/NodeScene.hpp"
#include "NodeModels/NetModel.hpp"
#include "toolgui/NodeMacros.h"
#include <set>
#include <stack>
#include <algorithm>
#include <unordered_map>
#include "optimizer/NLEquation.hpp"
#include "optimizer/DiffEquation.hpp"
#include "optimizer/Observer.hpp"
#include "optimizer/NLStatefulEquation.hpp"
#include "BlockClasses/BlockClassesManager.hpp"
#include "optimizer/NLDiffSolver.hpp"

struct Net
{
	int in_sockets_count = 0;
	int out_sockets_count = 0;
	std::vector<node::model::SocketUniqueId> sockets;
	std::vector<node::model::NetNodeId> nodes;
};

struct SocketMapping
{
	node::model::SocketId socket_id;
	bool connected;
	node::model::BlockSocketModel::SocketType socket_type;
	size_t net_id;
};

struct SocketMappings
{
	node::model::BlockId block_id;
	std::vector<SocketMapping> mappings;
};

struct NetSplitResult
{
	std::vector<Net> nets;
	std::unordered_map<node::model::SocketUniqueId, size_t> socket_mapping;
};

struct BlocksFunctions
{
	std::vector<std::pair<opt::NLEquation,node::model::BlockId>> nl_eqs;
	std::vector<std::pair<opt::NLStatefulEquation, node::model::BlockId>> nl_st_eqs;
	std::vector<std::pair<opt::DiffEquation, node::model::BlockId>> diff_eqs;
	std::vector<std::pair<opt::Observer, node::model::BlockId>> observers;
};

// helper type for the visitor #4
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;


static BlocksFunctions CreateBlocks(const node::model::NodeSceneModel& scene, node::BlockClassesManager& mgr)
{
	BlocksFunctions funcs;
	for (const auto& block : scene.GetBlocks())
	{
		auto block_class = mgr.GetBlockClassByName(block.GetClass());
		assert(block_class);
		auto functor = block_class->GetFunctor(block.GetProperties());
		std::visit(overloaded{
			[&](opt::NLEquation& eq) {funcs.nl_eqs.push_back({std::move(eq), block.GetId()}); },
			[&](opt::NLStatefulEquation& eq) {funcs.nl_st_eqs.push_back({std::move(eq), block.GetId()}); },
			[&](opt::DiffEquation& eq) {funcs.diff_eqs.push_back({std::move(eq), block.GetId()}); },
			[&](opt::Observer& eq) {funcs.observers.push_back({std::move(eq), block.GetId()}); }
			}, functor);
	}
	return funcs;
}

static NetSplitResult SplitToNets(const node::model::NodeSceneModel& scene)
{	
	using namespace node::model;
	NetSplitResult result;
	auto&& nets = result.nets;

	std::set<NetNodeId> unvisited_nodes;
	std::stack<NetNodeId> to_visit_nodes;
	for (auto&& node : scene.GetNetNodes())
	{
		unvisited_nodes.insert(node.GetId());
	}
	size_t current_net_id = 0;
	while (unvisited_nodes.size())
	{
		nets.push_back({});
		auto& net = nets.back();
		{
			auto node_id = *unvisited_nodes.begin();
			unvisited_nodes.erase(unvisited_nodes.begin());
			to_visit_nodes.push(std::move(node_id));
		}
		while (to_visit_nodes.size())
		{
			auto node_id = to_visit_nodes.top();
			to_visit_nodes.pop();
			net.nodes.push_back(node_id);
			auto node = scene.GetNetNodeById(node_id);
			assert(node);
			auto conn = scene.GetSocketConnectionForNode(node_id);
			if (conn)
			{
				result.socket_mapping.emplace(conn->get().socketId, current_net_id);
			}
			for (int i = 0; i < 4; i++)
			{
				auto segment_id = node->get().GetSegmentAt(static_cast<ConnectedSegmentSide>(i));
				if (segment_id)
				{
					auto segment = scene.GetNetSegmentById(*segment_id);
					assert(segment);
					auto node_id_next = segment->get().m_firstNodeId == node_id ? segment->get().m_secondNodeId : segment->get().m_firstNodeId;
					auto it = unvisited_nodes.find(node_id_next);
					if (it != unvisited_nodes.end())
					{
						to_visit_nodes.push(node_id_next);
						unvisited_nodes.erase(it);
					}
				}
			}
		}
		current_net_id += 1;

	}
	
	

	return result;
}


static std::vector<SocketMappings> MapSockets(const node::model::NodeSceneModel& scene, NetSplitResult& nets)
{
	using namespace node::model;
	auto&& socket_mapping = nets.socket_mapping;
	std::vector<SocketMappings> mappings;
	for (const auto& block : scene.GetBlocks())
	{
		auto block_id = block.GetId();
		mappings.push_back({ block_id });
		auto&& mapping = mappings.back();
		for (const auto& socket : block.GetSockets())
		{
			auto socket_id = socket.GetId();
			auto it = socket_mapping.find(SocketUniqueId{socket_id, block_id});
			if (it != socket_mapping.end())
			{
				nets.nets[it->second].sockets.push_back(SocketUniqueId{ socket_id, block_id });
				if (socket.GetType() == BlockSocketModel::SocketType::input)
				{
					nets.nets[it->second].in_sockets_count += 1;
				}
				else
				{
					nets.nets[it->second].out_sockets_count += 1;
				}
				mapping.mappings.push_back(SocketMapping{ socket_id, true, socket.GetType(), it->second });
			}
			else
			{
				mapping.mappings.push_back(SocketMapping{ socket_id, false, socket.GetType(), 0 });
			}
		}
	}
	return mappings;
}

static void RemapFunctions(BlocksFunctions& funcs, const std::vector<SocketMappings>& mappings)
{
	using namespace node::model;
	auto remap = [&](auto&& range)
		{
			for (auto& func_obj : range)
			{
				auto& block_id = func_obj.second;
				auto& func = func_obj.first;
				int socket_id = 0;
				int input_id = 0;
				int output_id = 0;
				auto it = std::find_if(mappings.begin(), mappings.end(), [&](const auto& obj)->bool {return obj.block_id == block_id; });
				assert(it != mappings.end());
				for (const auto& mapping : it->mappings)
				{
					auto value = mapping.net_id;
					if (!mapping.connected)
					{
						value = 9999;
					}
					if (mapping.socket_type == BlockSocketModel::SocketType::input)
					{
						func.get_input_ids()[input_id] = value;
						socket_id++;
						input_id++;
					}
					else
					{
						func.get_output_ids()[output_id] = value;
						socket_id++;
						output_id++;
					}
				}
			}
		};
	remap(funcs.nl_eqs);
	remap(funcs.nl_st_eqs);
	remap(funcs.diff_eqs);
	remap(funcs.observers);
}

static void AddFuncs(opt::NLDiffSolver& solver, BlocksFunctions& funcs)
{
	for (auto& func : funcs.diff_eqs)
	{
		solver.AddDiffEquation(std::move(func.first));
	}
	for (auto& func : funcs.nl_eqs)
	{
		solver.AddNLEquation(std::move(func.first));
	}
	for (auto& func : funcs.observers)
	{
		solver.AddObserver(std::move(func.first));
	}
	for (auto& func : funcs.nl_st_eqs)
	{
		solver.AddNLStatefulEquation(std::move(func.first));
	}
	
}
node::SimulatorRunner::SimulatorRunner(const model::NodeSceneModel& model, std::shared_ptr<BlockClassesManager> classes_mgr, std::function<void()> end_callback)
	:m_end_callback{ std::move(end_callback) }, m_model{std::make_unique<model::NodeSceneModel>(model)}, m_classes_mgr{std::move(classes_mgr)}
{
}

node::SimulatorRunner::~SimulatorRunner()
{
	if (!m_ended.test())
	{
		Stop();
	}
	if (m_thread.joinable())
	{
		m_thread.join();
	}
}

void node::SimulatorRunner::Run()
{
	m_thread = std::thread{ [this]() { this->RunImpl(); } };
}

void node::SimulatorRunner::Stop()
{
	m_stopped.test_and_set();
}

bool node::SimulatorRunner::IsEnded()
{
	return m_ended.test();
}

node::SimulationEvent node::SimulatorRunner::DoSimulation()
{
	auto nets = SplitToNets(*m_model);
	auto socket_mappings = MapSockets(*m_model, nets);

	assert(m_classes_mgr);
	auto blocks = CreateBlocks(*m_model, *m_classes_mgr);
	RemapFunctions(blocks, socket_mappings);

	for (auto& net : nets.nets)
	{
		if (net.in_sockets_count > 0 && net.out_sockets_count == 0)
		{
			return { SimulationEvent::NetFloatingError{ std::move(net.nodes) } };
		}
		if (net.out_sockets_count > 1)
		{
			std::vector<model::SocketUniqueId> output_sockets;
			for (const auto& socket_id : net.sockets)
			{
				if (auto block = m_model->GetBlockById(socket_id.block_id))
				{
					auto socket = block->get().GetSocketById(socket_id.socket_id);
					assert(socket);
					if (socket->get().GetType() == model::BlockSocketModel::SocketType::output)
					{
						output_sockets.push_back(socket_id);
					}
				}
			}
			return { SimulationEvent::OutputSocketsConflict{std::move(net.nodes), std::move(output_sockets)}};
		}
	}
	std::vector<model::SocketUniqueId> floating_inputs;
	for (const auto& mapping : socket_mappings)
	{
		for (const auto& socket : mapping.mappings)
		{
			if (socket.connected == false && socket.socket_type == model::BlockSocketModel::SocketType::input)
			{
				floating_inputs.push_back({ socket.socket_id, mapping.block_id });
			}
		}
	}
	if (floating_inputs.size())
	{
		return { SimulationEvent::FloatingInput{std::move(floating_inputs) } };
	}
	opt::NLDiffSolver solver;
	AddFuncs(solver, blocks);
	return { SimulationEvent::Success{} };
}

void node::SimulatorRunner::RunImpl()
{
	m_evt = DoSimulation();
	
	m_ended.test_and_set();
	if (m_end_callback)
	{
		m_end_callback();
	}
}
