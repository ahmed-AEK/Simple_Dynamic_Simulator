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
#include "PluginAPI/BlockClassesManager.hpp"
#include "optimizer/NLDiffSolver.hpp"
#include "optimizer/SourceEq.hpp"
#include "NodeModels/FunctionalBlocksDataManager.hpp"
#include <queue>

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
	std::unordered_map<int32_t, Net> nets;
	std::unordered_map<node::model::SocketUniqueId, int32_t> socket_mapping;
};

struct PortSimulationDescriptor
{
	node::model::SocketId socket_id;
	node::model::SocketType socket_type;
};

struct SubsystemSimulationDescriptor
{
	const node::model::SubsystemBlockData& subystem_data;
	int32_t subsystemRequestId;
	std::vector<std::pair<int32_t, PortSimulationDescriptor>> sockets;
};
struct PortBlockSimulationDescriptor
{
	int32_t net_id;
	PortSimulationDescriptor port_data;
};

struct BlocksFunctions
{
	std::vector<std::pair<opt::NLEquationWrapper,node::model::BlockId>> nl_eqs;
	std::vector<std::pair<opt::NLStatefulEquationWrapper, node::model::BlockId>> nl_st_eqs;
	std::vector<std::pair<opt::DiffEquationWrapper, node::model::BlockId>> diff_eqs;
	std::vector<std::pair<opt::ObserverWrapper, node::model::BlockId>> observers;
	std::vector<std::pair<opt::SourceEqWrapper, node::model::BlockId>> sources;
	std::vector<std::pair<SubsystemSimulationDescriptor, node::model::BlockId>> subsystem_blocks;
	std::vector<std::pair<PortBlockSimulationDescriptor, node::model::BlockId>> ports_blocks;
};

struct SimulationSubsystemsManager
{
	struct SimulationSubsystemData
	{
		int32_t request_id;
		std::reference_wrapper<const node::model::SubsystemBlockData> subsystem_data;
	};
	std::vector<SimulationSubsystemData> simulationSubsystemsMapping;
	std::queue<int32_t> remaining_block_requests;
	int32_t next_block_id = 0;

	SimulationSubsystemsManager::SimulationSubsystemData* GetRequestData(int32_t request_id)
	{
		auto it = std::find_if(simulationSubsystemsMapping.begin(),
			simulationSubsystemsMapping.end(),
			[&](const auto& mapping) {return mapping.request_id == request_id; });
		if (it != simulationSubsystemsMapping.end())
		{
			return &(*it);
		}
		return nullptr;
	}

	int32_t AddRequest(const node::model::SubsystemBlockData& subsystem_data)
	{
		int32_t request_id = next_block_id;
		simulationSubsystemsMapping.push_back({ request_id, subsystem_data });
		remaining_block_requests.push(request_id);
		next_block_id++;
		return request_id;
	}
	std::optional<int32_t> GetNextRequest()
	{
		if (remaining_block_requests.size())
		{
			auto id = remaining_block_requests.front();
			remaining_block_requests.pop();
			return id;
		}
		return std::nullopt;
	}
};

// helper type for the visitor #4
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;


static [[nodiscard]] tl::expected<std::monostate, std::string> AddFunctionalBlock(node::BlockClassesManager& mgr, 
	BlocksFunctions& funcs, const node::model::FunctionalBlocksDataManager& functionalBlocksManager, 
	const node::model::BlockModel& block)
{
	auto block_data_ptr = functionalBlocksManager.GetDataForId(block.GetId());
	assert(block_data_ptr);
	auto block_class = mgr.GetBlockClassByName(block_data_ptr->block_class);
	assert(block_class);
	auto functor = block_class->GetFunctor(block_data_ptr->properties);

	auto block_adder = [&](node::BlockFunctor& functor) {
		std::visit(overloaded{
		[&](opt::NLEquationWrapper& eq) {funcs.nl_eqs.push_back({std::move(eq), block.GetId()}); },
		[&](opt::NLStatefulEquationWrapper& eq) {funcs.nl_st_eqs.push_back({std::move(eq), block.GetId()}); },
		[&](opt::DiffEquationWrapper& eq) {funcs.diff_eqs.push_back({std::move(eq), block.GetId()}); },
		[&](opt::ObserverWrapper& eq) {funcs.observers.push_back({std::move(eq), block.GetId()}); },
		[&](opt::SourceEqWrapper& eq) {funcs.sources.push_back({std::move(eq), block.GetId()}); }
			}, functor);
		};
	if (auto* err = std::get_if<std::string>(&functor))
	{
		return tl::unexpected<std::string>{std::move(*err)};
	}

	std::visit(overloaded{
		block_adder,
		[&](std::vector<node::BlockFunctor>& blocks)
		{
			for (auto& block : blocks)
			{
				block_adder(block);
			}
		} ,
		[](const std::string&) { assert(false); /* already checked */}
		}, functor);
	return std::monostate{};
}

static void AddSubSystemBlock(BlocksFunctions& funcs, 
	const node::model::SubsystemBlocksDataManager& subsystemBlocksManager,
	SimulationSubsystemsManager& simulationSubsystemsManager,
	const node::model::BlockModel& block)
{
	using namespace node;
	auto* subsystem_data_ptr = subsystemBlocksManager.GetDataForId(block.GetId());
	assert(subsystem_data_ptr);
	std::vector<std::pair<int32_t, PortSimulationDescriptor>> sockets;
	sockets.reserve(block.GetSockets().size());
	for (auto&& socket : block.GetSockets())
	{
		sockets.push_back({ socket.GetId().value, PortSimulationDescriptor{ socket.GetId(), socket.GetType() } });
	}
	auto request_id = simulationSubsystemsManager.AddRequest(*subsystem_data_ptr);
	funcs.subsystem_blocks.push_back({ 
		SubsystemSimulationDescriptor{*subsystem_data_ptr, request_id, std::move(sockets)}, 
		block.GetId() });
}


static void AddPortBlock(BlocksFunctions& funcs,
	const node::model::PortBlocksDataManager& PortBlocksManager,
	const node::model::BlockModel& block)
{
	using namespace node;
	auto* port_data_ptr = PortBlocksManager.GetDataForId(block.GetId());
	assert(port_data_ptr);
	funcs.ports_blocks.push_back({ 
		PortBlockSimulationDescriptor{0,
		PortSimulationDescriptor{port_data_ptr->id, port_data_ptr->port_type}},
		block.GetId()});
}

static [[nodiscard]] tl::expected<BlocksFunctions, std::string> CreateBlocks(const node::model::NodeSceneModel& scene, node::BlockClassesManager& mgr,
	SimulationSubsystemsManager& simulationSubsystemsManager)
{
	/*
	goes over all blocks
	creates simulator functions and subsystems and ports 
	those will be later mapped to nets in other functions
	*/
	using namespace node;
	BlocksFunctions funcs;
	using namespace node;
	auto& functionalBlocksManager = scene.GetFunctionalBlocksManager();
	auto& subsystemBlocksManager = scene.GetSubsystemBlocksManager();
	auto& portBlocksManager = scene.GetPortBlocksManager();
	for (const auto& block : scene.GetBlocks())
	{
		if (block.GetType() == model::BlockType::Functional)
		{
			auto res = AddFunctionalBlock(mgr, funcs, functionalBlocksManager, block);
			if (!res)
			{
				return tl::unexpected{ std::move(res.error()) };
			}
		}
		else if (block.GetType() == model::BlockType::SubSystem)
		{
			AddSubSystemBlock(funcs, subsystemBlocksManager, simulationSubsystemsManager, block);
		}
		else if (block.GetType() == model::BlockType::Port)
		{
			AddPortBlock(funcs, portBlocksManager, block);
		}
		else
		{
			assert(false); // other modes not supported!
		}

	}
	return funcs;
}

static NetSplitResult SplitToNets(const node::model::NodeSceneModel& scene, int32_t& next_net_id)
{	
	/*
	goes over all nodes
	combines nodes into nets, with net_id
	maps socket_id to net_id

	algorithm:
	add all nodes to unvisited nodes
	starts from the net nodes at a source in to_visit_nodes
	collect connected socket in socket_mapping with its net id and add it to Net
	for each of the 4 connected nodes:
	if they are in unvisited, pop them, and add them to to_visit_nodes
	repeat till to_visit_nodes is empty

	repeat for other nets

	socket_mapping: socket_id -> net_id
	Net: net_id -> nodes.
	sockets are not added to Net yet, 
	because we only visit node to socket connections, not sockets
	*/
	using namespace node::model;
	NetSplitResult result;
	auto&& nets = result.nets;

	std::set<NetNodeId> unvisited_nodes;
	std::stack<NetNodeId> to_visit_nodes;
	for (auto&& node : scene.GetNetNodes())
	{
		unvisited_nodes.insert(node.GetId());
	}
	while (unvisited_nodes.size())
	{
		auto& net = nets[next_net_id];
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
				result.socket_mapping.emplace(conn->socketId, next_net_id);
			}
			for (int i = 0; i < 4; i++)
			{
				auto segment_id = node->GetSegmentAt(static_cast<ConnectedSegmentSide>(i));
				if (segment_id)
				{
					auto segment = scene.GetNetSegmentById(*segment_id);
					assert(segment);
					auto node_id_next = segment->m_firstNodeId == node_id ? segment->m_secondNodeId : segment->m_firstNodeId;
					auto it = unvisited_nodes.find(node_id_next);
					if (it != unvisited_nodes.end())
					{
						to_visit_nodes.push(node_id_next);
						unvisited_nodes.erase(it);
					}
				}
			}
		}
		next_net_id += 1;
	}
	
	

	return result;
}


static std::vector<SocketMappings> MapSockets(const node::model::NodeSceneModel& scene, NetSplitResult& nets)
{
	/*
	go over all blocks
	uses the map of socket_id to net_id to fill Net with socket
	and fill SocketMappings, which maps block -> Nets

	for each socket, increment the Net input and output sockets counts
	for each socket, add it to the Net, with its type now known
	for each socket, add it to the SocketMappings of this block

	Net: net_id -> SocketIds and counts for debugging
	SocketMappings: Block -> net_ids
	*/
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

static void RemapFunctions(BlocksFunctions& funcs, 
	const std::vector<SocketMappings>& mappings, int32_t& max_net)
{
	/*
	goes over all created functions.

	uses the SocketMappings to map SocketId -> net_id
	*/
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
				if constexpr (requires {func.input_ids; })
				{
					for (auto& id : func.input_ids)
					{
						id_mapper(id);
					}
				}
				if constexpr (requires {func.output_ids; })
				{
					for (auto& id : func.output_ids)
					{
						id_mapper(id);
					}
				}
				if constexpr (requires {func.sockets; })
				{
					for (auto& [socket_id, socket_data] : func.sockets)
					{
						id_mapper(socket_id);
					}
				}
				if constexpr (requires {func.net_id; })
				{
					id_mapper(func.net_id);
				}
			}
		};
	remap(funcs.nl_eqs);
	remap(funcs.nl_st_eqs);
	remap(funcs.diff_eqs);
	remap(funcs.observers);
	remap(funcs.sources);
	remap(funcs.subsystem_blocks);
	remap(funcs.ports_blocks);
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
		auto& out_ids = func.first.output_ids;
		if (out_ids.size() && out_ids[0] == 9999)
		{
			continue;
		}
		solver.AddDiffEquation(std::move(func.first));
	}
	for (auto& func : funcs.nl_eqs)
	{
		auto& out_ids = func.first.output_ids;
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
		auto& out_ids = func.first.output_ids;
		if (out_ids.size() && out_ids[0] == 9999)
		{
			continue;
		}
		solver.AddNLStatefulEquation(std::move(func.first));
	}
	for (auto& func : funcs.sources)
	{
		auto& out_ids = func.first.output_ids;
		if (out_ids.size() && out_ids[0] == 9999)
		{
			continue;
		}
		solver.AddSource(std::move(func.first));
	}
	return result;
}

static void AddPortsToSolver(opt::NLDiffSolver& solver,
	const std::unordered_map<int32_t, BlocksFunctions>& subsystems)
{
	/*
	loops over all subsystems
	looks into each contained subsystem block
	looks for its ports in another subsystem
	adds buffer nodes between the subsystem block sockets nets and the port nets
	*/
	using namespace node;

	for (auto&& [request_id, subsystem] : subsystems)
	{
		for (auto&& [subsystem_data, block_id] : subsystem.subsystem_blocks)
		{
			auto it_target_subsystem = subsystems.find(subsystem_data.subsystemRequestId);
			assert(it_target_subsystem != subsystems.end());

			for (auto&& [net_id, socket_data] : subsystem_data.sockets)
			{
				auto& port_blocks = it_target_subsystem->second.ports_blocks;
				auto socket_it = std::find_if(port_blocks.begin(), port_blocks.end(),
					[&](const std::pair<PortBlockSimulationDescriptor, node::model::BlockId>& obj) 
					{ return obj.first.port_data.socket_id == socket_data.socket_id; });
				assert(socket_it != port_blocks.end());
				if (socket_data.socket_type == model::SocketType::input)
				{
					solver.AddBufferEquation({ net_id, socket_it->first.net_id });
				}
				else
				{
					solver.AddBufferEquation({ socket_it->first.net_id, net_id });
				}
			}
		}
	}
}

static std::vector<std::unique_ptr<node::model::NodeSceneModel>> CopyScenes(
	std::vector<std::reference_wrapper<const node::model::NodeSceneModel>> models)
{
	using namespace node;
	std::vector<std::unique_ptr<node::model::NodeSceneModel>> out_models;
	out_models.reserve(models.size());
	for (auto&& model : models)
	{
		out_models.push_back(std::make_unique<model::NodeSceneModel>(model.get()));
	}
	return out_models;
}

node::SimulatorRunner::SimulatorRunner(
	std::vector<std::reference_wrapper<const model::NodeSceneModel>> models, 
	SubSceneId main_subscene_id,
	std::shared_ptr<BlockClassesManager> classes_mgr, std::function<void()> end_callback, 
	SimulationSettings settings)
	:m_end_callback{ std::move(end_callback) }, m_main_subscene_id{ main_subscene_id },
	m_models{CopyScenes(models)},
	m_classes_mgr{std::move(classes_mgr)}, m_settings{settings}
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
	/*
	goes over all nets
	makes sure outputs = 1 if inputs > 0
	errors when outputs > 1
	errors when ouputs = 0 if inputs > 0
	*/
	using namespace node;
	for (auto& [net_id, net] : nets.nets)
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
				if (auto* block = model.GetBlockById(socket_id.block_id))
				{
					auto socket = block->GetSocketById(socket_id.socket_id);
					assert(socket);
					if (socket->GetType() == model::BlockSocketModel::SocketType::output)
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
	SimulationSubsystemsManager simulationSubsystemsManager;
	std::unordered_map<int32_t, BlocksFunctions> subsystems;
	model::SubsystemBlockData main_block_subsystem{ "Local", m_main_subscene_id };
	simulationSubsystemsManager.AddRequest(main_block_subsystem);
	int32_t next_net_id = 0;
	while (auto next_request = simulationSubsystemsManager.GetNextRequest())
	{
		assert(simulationSubsystemsManager.GetRequestData(*next_request));
		auto* next_request_data = simulationSubsystemsManager.GetRequestData(*next_request);

		assert(next_request_data->subsystem_data.get().URL == "Local");
		auto model_it = std::find_if(m_models.begin(), m_models.end(),
			[&](const std::unique_ptr<model::NodeSceneModel>& model)
			{return model->GetSubSceneId() == next_request_data->subsystem_data.get().scene_id; });
		if (model_it == m_models.end())
		{
			return SimulationEvent{ SimulationEvent::RequestedBadScene{ next_request_data->subsystem_data.get().scene_id } };
		}

		auto nets = SplitToNets(**model_it, next_net_id);
		auto socket_mappings = MapSockets(**model_it, nets);

		assert(m_classes_mgr);
		auto blocks = CreateBlocks(**model_it, *m_classes_mgr, simulationSubsystemsManager);
		if (!blocks)
		{
			return { SimulationEvent::SimulationError{ std::move(blocks.error()) } };
		}
		RemapFunctions(*blocks, socket_mappings, next_net_id);

		auto validation_result = ValidateNets(nets, socket_mappings, **model_it);
		if (validation_result)
		{
			return std::visit([](auto& val) {return SimulationEvent{ std::move(val) }; }, *validation_result);
		}

		subsystems.emplace(*next_request, std::move(*blocks));
	}
	
	opt::NLDiffSolver solver;
	std::vector<ObserverMapping> observer_mapping;
	for (auto& [request_id, subsystem_blocks] : subsystems)
	{
		auto observer_mapping_local = AddFuncs(solver, subsystem_blocks);
		for (auto&& observer_mapping_obj : observer_mapping_local)
		{
			observer_mapping.push_back(std::move(observer_mapping_obj));
		}
	}
	AddPortsToSolver(solver, subsystems);
	
	solver.SetMaxStep(m_settings.max_step);
	solver.Initialize(m_settings.t_start, m_settings.t_end);

	opt::FlatMap simulation_nets{ next_net_id };
	opt::StepEnd step_result = opt::StepEnd::Success;
	
	{
		auto initial_result = solver.CalculateInitialConditions(simulation_nets);
		if (!initial_result)
		{
			return { SimulationEvent::SimulationError{std::move(initial_result.error())} };
		}
	}

	while (step_result != opt::StepEnd::ReachedEnd && !m_stopped.load())
	{
		auto result_temp = solver.Step(simulation_nets);
		if (!result_temp)
		{
			return { SimulationEvent::SimulationError{std::move(result_temp.error())} };
		}
		step_result = *result_temp;
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
