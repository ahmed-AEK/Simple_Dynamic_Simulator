#include "LuaStandaloneStatefulEqnClass.hpp"
#include "sol/sol.hpp"
#include "PluginAPI/Logger.hpp"
#include "PluginAPI/BlockDialog.hpp"
#include "LuaPlugin/LuaEditorDialog.hpp"
#include "LuaPlugin/LuaStatefulEqn.hpp"
#include <sstream>

static const std::vector<node::model::BlockProperty> ClassProperties{
	*node::model::BlockProperty::Create("Inputs Count", node::model::BlockPropertyType::UnsignedInteger, 0.0),
	*node::model::BlockProperty::Create("Code", node::model::BlockPropertyType::String, "Here Should be some code!"),
};

static constexpr std::string_view Description = "Parses Simple expressions using lua, inputs: a,b,c,d,e,f and t for time";

node::LuaStandaloneStatefulEqnClass::LuaStandaloneStatefulEqnClass()
	:node::BlockClass{""}
{
}

void node::LuaStandaloneStatefulEqnClass::GetName(GetNameCallback cb, void* context) const
{
	static constexpr std::string_view name = "LuaStandaloneStatefulEqn";
	cb(context, name);
}

void node::LuaStandaloneStatefulEqnClass::GetDescription(GetDescriptionCallback cb, void* context) const
{
	cb(context, Description);
}

void node::LuaStandaloneStatefulEqnClass::GetDefaultClassProperties(GetDefaultClassPropertiesCallback cb, void* context) const
{
	cb(context, ClassProperties);
}

int node::LuaStandaloneStatefulEqnClass::ValidateClassProperties(std::span<const model::BlockProperty> properties, IValidatePropertiesNotifier& error_cb) const
{
	if (properties.size() != ClassProperties.size())
	{
		error_cb.error(0, std::format("size mismatch, expected: {}, got: {}", ClassProperties.size(), properties.size()));
		return false;
	}
	for (size_t i = 0; i < properties.size(); i++)
	{
		if (properties[i].name != ClassProperties[i].name)
		{
			error_cb.error(i, std::format("property name mismatch, expected: {}, got: {}", ClassProperties[i].name, properties[i].name));
			return false;
		}
		if (properties[i].GetType() != ClassProperties[i].GetType())
		{
			error_cb.error(i, std::format("property type mismatch"));
			return false;
		}
	}
	auto* in_sockets_count = properties[0].get_uint();
	assert(in_sockets_count);
	if (*in_sockets_count > 6)
	{
		error_cb.error(0, std::format("only 6 input sockets allowed"));
		return false;
	}
	auto* code = properties[1].get_str();
	assert(code);
	if (!code || !code->size())
	{
		error_cb.error(1, std::format("no code found"));
		return false;
	}
	auto validation_result = validate_lua_script(*code);
	if (!validation_result)
	{
		error_cb.error(1, validation_result.error());
		return false;
	}
	return true;
}

void node::LuaStandaloneStatefulEqnClass::CalculateSockets(std::span<const model::BlockProperty> properties, CalculateSocketCallback cb, void* context) const
{
	using model::SocketType;
	if (properties.size() < 2)
	{
		assert(false);
		return;
	}

	auto* in_sockets_count = properties[0].get_uint();
	if (!in_sockets_count)
	{
		assert(false);
		return;
	}

	std::vector<SocketType> sockets;
	sockets.reserve(1 + *in_sockets_count);
	for (size_t i = 0; i < *in_sockets_count; i++)
	{
		sockets.push_back(SocketType::input);
	}
	sockets.push_back(SocketType::output);
	cb(context, sockets);
}

node::BlockType node::LuaStandaloneStatefulEqnClass::GetBlockType(std::span<const model::BlockProperty> properties) const
{
	UNUSED_PARAM(properties);
	return BlockType::Stateful;
}

int node::LuaStandaloneStatefulEqnClass::GetFunctor(std::span<const model::BlockProperty> properties, IGetFunctorCallback& cb) const
{	
	[[maybe_unused]] LightValidatePropertiesNotifier notifier;
	auto valid = ValidateClassProperties(properties, notifier);
	if (notifier.errored || !valid)
	{
		cb.error("failed to validate properties");
		return false;
	}

	auto* in_sockets_count = properties[0].get_uint();
	assert(in_sockets_count);
	sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::math);
	auto* code = properties[1].get_str();
	assert(code);

	lua::NLStatefulEqnBuilder builder{ m_logger };

	auto funcs = builder.build_lua_functions(*code, lua);
	if (!funcs)
	{
		cb.error(funcs.error());
		return false;
	}

	std::vector<int32_t> inputs;
	inputs.reserve(*in_sockets_count);
	for (size_t i = 0; i < *in_sockets_count; i++)
	{
		inputs.push_back(static_cast<int32_t>(i));
	}
	opt::NLStatefulEquationWrapper eq{
		std::move(inputs),
		{static_cast<int32_t>(*in_sockets_count)},
		opt::make_NLStatefulEqn<lua::NLStatefulEqn>(std::move(lua), std::move(*funcs)),
		{}
	};
	node::BlockView view{ eq };
	cb.call({ &view,1 });
	return true;
}

bool node::LuaStandaloneStatefulEqnClass::HasBlockDialog() const
{
	return true;
}

static bool ValidateCanOpenEditor(node::model::FunctionalBlockData& data, node::logging::Logger& logger)
{
	if (data.properties.size() < 2)
	{
		logger.LogError("cannot open editor, expected 2 properties, found: {}", data.properties.size());
		return false;
	}
	if (data.properties[1].GetType() != node::model::BlockPropertyType::String)
	{
		logger.LogError("cannot open editor, expected string property, found: {}", data.properties[1].GetTypeAsString());
		return false;
	}
	return true;
}

std::unique_ptr<node::BlockDialog> node::LuaStandaloneStatefulEqnClass::CreateBlockDialog(Scene& scene, std::shared_ptr<IBlockPropertiesUpdater> model_updater, model::BlockModel& model, model::FunctionalBlockData& data, std::any& simulation_data)
{
	UNUSED_PARAM(data);
	UNUSED_PARAM(simulation_data);
	if (!ValidateCanOpenEditor(data, m_logger))
	{
		return {};
	}

	return LuaEditorDialog::Create(model, std::move(model_updater), BlockClassPtr{BlockClassPtr::new_reference,this}, "Code", WidgetSize{500.0f,500.0f}, &scene);
}

tl::expected<std::monostate, std::string> node::LuaStandaloneStatefulEqnClass::validate_lua_script(const std::string& code) const
{
	lua::NLStatefulEqnBuilder builder{ m_logger };

	sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::math);
	auto expr_opt = builder.build_lua_functions(code, lua);
	if (!expr_opt)
	{
		return tl::unexpected<std::string>{std::move(expr_opt.error())};
	}

	return std::monostate{};
}
