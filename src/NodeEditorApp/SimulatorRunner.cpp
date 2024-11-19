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
#include "optimizer/SourceEq.hpp"

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
	int32_t net_id;
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
	std::vector<std::pair<opt::SourceEq, node::model::BlockId>> sources;
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

		auto block_adder = [&](node::BlockFunctor& functor) {
			std::visit(overloaded{
			[&](opt::NLEquation& eq) {funcs.nl_eqs.push_back({std::move(eq), block.GetId()}); },
			[&](opt::NLStatefulEquation& eq) {funcs.nl_st_eqs.push_back({std::move(eq), block.GetId()}); },
			[&](opt::DiffEquation& eq) {funcs.diff_eqs.push_back({std::move(eq), block.GetId()}); },
			[&](opt::Observer& eq) {funcs.observers.push_back({std::move(eq), block.GetId()}); },
			[&](opt::SourceEq& eq) {funcs.sources.push_back({std::move(eq), block.GetId()}); }
				}, functor);
			};

		std::visit(overloaded{ 
			block_adder, 
			[&](std::vector<node::BlockFunctor>& blocks) 
			{
				for (auto& block : blocks)
				{
					block_adder(block);
				}
			} }, functor);
		
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
		mappings.push_back(SocketMappings{ block_id, {} });
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
				mapping.mappings.push_back(SocketMapping{ socket_id, true, socket.GetType(), static_cast<int32_t>(it->second) });
			}
			else
			{
				mapping.mappings.push_back(SocketMapping{ socket_id, false, socket.GetType(), 0 });
			}
		}
	}
	return mappings;
}

static void RemapFunctions(BlocksFunctions& funcs, const std::vector<SocketMappings>& mappings, int32_t& max_net)
{
	using namespace node::model;
	struct FunctorTerminalsMapping
	{
		int32_t initial_value;
		int32_t transformed_value;
	};

	std::unordered_map<BlockId, std::vector<FunctorTerminalsMapping>> functors_mapping;
	
	auto remap = [&](auto&& range)
		{
			for (auto& func_obj : range)
			{
				auto& block_id = func_obj.second;
				auto& func = func_obj.first;
				auto it = std::find_if(mappings.begin(), mappings.end(), [&](const auto& obj)->bool {return obj.block_id == block_id; });
				assert(it != mappings.end());
				auto id_mapper = [&](int32_t& id)
					{
						// look for the socket id in the scene sockets ids, if it is a block socket it should be there, otherwise it is an internal node
						auto socket_it = std::find_if(it->mappings.begin(), it->mappings.end(), [&](const SocketMapping& mapping) {return mapping.socket_id.value == id; });
						if (socket_it != it->mappings.end())
						{
							// if it is part of the nets mapping (block socket), map it if it is connected.
							if (!it->mappings[id].connected)
							{
								functors_mapping[block_id].push_back(FunctorTerminalsMapping{ static_cast<int32_t>(id), 9999 });
								id = 9999;
							}
							else
							{
								functors_mapping[block_id].push_back(FunctorTerminalsMapping{ static_cast<int32_t>(id), static_cast<int32_t>(socket_it->net_id) });
								id = socket_it->net_id;
							}
						}
						else
						{
							// see if we have seen it before, if we have then assign the same net_id, otherwise assign a new net_id to it.
							auto& mapping = functors_mapping[block_id];
							auto mapping_it = std::find_if(mapping.begin(), mapping.end(), [&](const FunctorTerminalsMapping& term) {return term.initial_value == static_cast<int32_t>(id); });
							if (mapping_it != mapping.end())
							{
								id = mapping_it->transformed_value;
							}
							else
							{
								functors_mapping[block_id].push_back(FunctorTerminalsMapping{ static_cast<int32_t>(id), max_net });
								id = max_net;
								max_net++;
							}
						}
					};
				if constexpr (requires {func.get_input_ids(); })
				{
					for (auto& id : func.get_input_ids())
					{
						id_mapper(id);
					}
				}
				if constexpr (requires {func.get_output_ids(); })
				{
					for (auto& id : func.get_output_ids())
					{
						id_mapper(id);
					}
				}
			}
		};
	remap(funcs.nl_eqs);
	remap(funcs.nl_st_eqs);
	remap(funcs.diff_eqs);
	remap(funcs.observers);
	remap(funcs.sources);
}

struct ObserverMapping
{
	size_t observer_id;
	node::model::BlockId block_id;
};

static std::vector<ObserverMapping> AddFuncs(opt::NLDiffSolver& solver, BlocksFunctions& funcs)
{
	std::vector<ObserverMapping> result;

	for (auto& func : funcs.diff_eqs)
	{
		auto out_ids = func.first.get_output_ids();
		if (out_ids.size() && out_ids[0] == 9999)
		{
			continue;
		}
		solver.AddDiffEquation(std::move(func.first));
	}
	for (auto& func : funcs.nl_eqs)
	{
		auto out_ids = func.first.get_output_ids();
		if (out_ids.size() && out_ids[0] == 9999)
		{
			continue;
		}
		solver.AddNLEquation(std::move(func.first));
	}
	for (auto& func : funcs.observers)
	{
		auto obs_id = solver.AddObserver(std::move(func.first));
		result.emplace_back(obs_id, func.second);
	}
	for (auto& func : funcs.nl_st_eqs)
	{
		auto out_ids = func.first.get_output_ids();
		if (out_ids.size() && out_ids[0] == 9999)
		{
			continue;
		}
		solver.AddNLStatefulEquation(std::move(func.first));
	}
	for (auto& func : funcs.sources)
	{
		auto out_ids = func.first.get_output_ids();
		if (out_ids.size() && out_ids[0] == 9999)
		{
			continue;
		}
		solver.AddSource(std::move(func.first));
	}
	return result;
}
node::SimulatorRunner::SimulatorRunner(const model::NodeSceneModel& model, std::shared_ptr<BlockClassesManager> classes_mgr, std::function<void()> end_callback, SimulationSettings settings)
	:m_end_callback{ std::move(end_callback) }, m_model{std::make_unique<model::NodeSceneModel>(model)}, m_classes_mgr{std::move(classes_mgr)}, m_settings{settings}
{
}

node::SimulatorRunner::~SimulatorRunner()
{
	if (!m_ended.load())
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
	m_stopped.exchange(true);
}

bool node::SimulatorRunner::IsEnded()
{
	return m_ended.load();
}

bool node::SimulatorRunner::IsStopped()
{
	return m_stopped.load();
}

using ErrorsVariant = std::variant<node::SimulationEvent::NetFloatingError, node::SimulationEvent::OutputSocketsConflict, node::SimulationEvent::FloatingInput>;
using ValidationResult = std::optional<ErrorsVariant>;

static ValidationResult ValidateNets(const NetSplitResult& nets, const std::vector<SocketMappings>& socket_mappings, const node::model::NodeSceneModel& model)
{
	using namespace node;
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
				if (auto block = model.GetBlockById(socket_id.block_id))
				{
					auto socket = block->get().GetSocketById(socket_id.socket_id);
					assert(socket);
					if (socket->get().GetType() == model::BlockSocketModel::SocketType::output)
					{
						output_sockets.push_back(socket_id);
					}
				}
			}
			return { SimulationEvent::OutputSocketsConflict{std::move(net.nodes), std::move(output_sockets)} };
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
	return std::nullopt;
}
node::SimulationEvent node::SimulatorRunner::DoSimulation()
{
	auto nets = SplitToNets(*m_model);
	auto socket_mappings = MapSockets(*m_model, nets);

	assert(m_classes_mgr);
	auto blocks = CreateBlocks(*m_model, *m_classes_mgr);
	int32_t max_net = static_cast<int32_t>(nets.nets.size());
	RemapFunctions(blocks, socket_mappings, max_net);
	
	assert(m_model);
	auto validation_result = ValidateNets(nets, socket_mappings, *m_model);
	if (validation_result)
	{
		return std::visit([](auto& val) {return SimulationEvent{std::move(val)}; }, *validation_result);
	}
	
	opt::NLDiffSolver solver;
	auto observer_mapping = AddFuncs(solver, blocks);
	
	solver.SetMaxStep(m_settings.max_step);
	solver.Initialize(m_settings.t_start, m_settings.t_end);

	opt::FlatMap simulation_nets{ max_net };
	opt::StepResult step_result = opt::StepResult::Success;
	
	solver.CalculateInitialConditions(simulation_nets);

	while (step_result != opt::StepResult::ReachedEnd && !m_stopped.load())
	{
		step_result = solver.Step(simulation_nets);
	}
	
	if (m_stopped.load())
	{
		return { SimulationEvent::Stopped{} };
	}

	auto simulation_result = solver.GetObserversData();
	std::vector<BlockResult> result;
	for (auto&& item : simulation_result)
	{
		auto it = std::find_if(observer_mapping.begin(), observer_mapping.end(),
			[&](const ObserverMapping& mapping) {return mapping.observer_id == item.id; });
		assert(it != observer_mapping.end());
		if (it != observer_mapping.end())
		{
			result.emplace_back(it->block_id, item.data);
		}
	}
 	return { SimulationEvent::Success{result} };
}

void node::SimulatorRunner::RunImpl()
{
	m_evt = DoSimulation();
	
	m_ended.exchange(true);
	if (m_end_callback)
	{
		m_end_callback();
	}
}
