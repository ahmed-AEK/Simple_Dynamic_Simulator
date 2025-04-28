#include "LuaStandaloneDiffEqnClass.hpp"
#include "sol/sol.hpp"
#include "PluginAPI/Logger.hpp"
#include "PluginAPI/BlockDialog.hpp"
#include "LuaPlugin/LuaEditorDialog.hpp"
#include "LuaPlugin/LuaDiffEqn.hpp"
#include <sstream>

const std::string_view node::LuaStandaloneDiffEqnClass::DEFAULT_CODE = R"(
function apply(input, output, t)
    output[1] = input[1]
end
)";


static const std::vector<node::model::BlockProperty> ClassProperties{
	*node::model::BlockProperty::Create("Inputs Count", node::model::BlockPropertyType::UnsignedInteger, 1),
	*node::model::BlockProperty::Create("Outputs Count", node::model::BlockPropertyType::UnsignedInteger, 1),
	*node::model::BlockProperty::Create("Code", node::model::BlockPropertyType::String, std::string{node::LuaStandaloneDiffEqnClass::DEFAULT_CODE}),
};

static constexpr std::string_view Description = "Parses the Lua code into a NLEqn";

node::LuaStandaloneDiffEqnClass::LuaStandaloneDiffEqnClass()
	:node::BlockClass{""}
{
}

void node::LuaStandaloneDiffEqnClass::GetName(GetNameCallback cb, void* context) const
{
	static constexpr std::string_view name = "LuaStandaloneDiffEqn";
	cb(context, name);
}

void node::LuaStandaloneDiffEqnClass::GetDescription(GetDescriptionCallback cb, void* context) const
{
	cb(context, Description);
}

void node::LuaStandaloneDiffEqnClass::GetDefaultClassProperties(GetDefaultClassPropertiesCallback cb, void* context) const
{
	cb(context, ClassProperties);
}

int node::LuaStandaloneDiffEqnClass::ValidateClassProperties(std::span<const model::BlockProperty> properties, IValidatePropertiesNotifier& error_cb) const
{
	if (!ValidateEqualPropertyTypes(properties, ClassProperties, error_cb))
	{
		return false;
	}
	auto* in_sockets_count = properties[0].get_uint();
	assert(in_sockets_count);
	if (*in_sockets_count > 6)
	{
		error_cb.error(0, std::format("only 6 input sockets allowed"));
		return false;
	}
	auto* out_sockets_count = properties[1].get_uint();
	if (*out_sockets_count > 6)
	{
		error_cb.error(0, std::format("only 6 output sockets allowed"));
		return false;
	}
	auto* code = properties[2].get_str();
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

void node::LuaStandaloneDiffEqnClass::CalculateSockets(std::span<const model::BlockProperty> properties, CalculateSocketCallback cb, void* context) const
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

	auto* out_sockets_count = properties[1].get_uint();
	if (!in_sockets_count)
	{
		assert(false);
		return;
	}

	std::vector<SocketType> sockets;
	sockets.reserve(*out_sockets_count + *in_sockets_count);
	for (size_t i = 0; i < *in_sockets_count; i++)
	{
		sockets.push_back(SocketType::input);
	}
	for (size_t i = 0; i < *out_sockets_count; i++)
	{
		sockets.push_back(SocketType::output);
	}
	cb(context, sockets);
}

node::BlockType node::LuaStandaloneDiffEqnClass::GetBlockType(std::span<const model::BlockProperty> properties) const
{
	UNUSED_PARAM(properties);
	return BlockType::Stateful;
}

int node::LuaStandaloneDiffEqnClass::GetFunctor(std::span<const model::BlockProperty> properties, IGetFunctorCallback& cb) const
{	
	[[maybe_unused]] LightValidatePropertiesNotifier notifier;
	auto valid = ValidateClassProperties(properties, notifier);
	if (notifier.errored || !valid)
	{
		cb.error("failed to validate properties");
		return false;
	}

	auto* in_sockets_count = properties[0].get_uint();
	auto* out_sockets_count = properties[1].get_uint();

	sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::math);
	auto* code = properties[2].get_str();
	assert(code);

	lua::DiffEqnBuilder builder{ m_logger };
	builder.AddUserTypes(lua);

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
	std::vector<int32_t> outputs;
	outputs.reserve(*out_sockets_count);
	for (size_t i = *in_sockets_count; i < *out_sockets_count + *in_sockets_count; i++)
	{
		outputs.push_back(static_cast<int32_t>(i));
	}
	opt::DiffEquationWrapper eq{
		std::move(inputs),
		std::move(outputs),
		opt::make_DiffEqn<lua::DiffEqn>(std::move(lua), std::move(*funcs)),
	};

	node::BlockView view{ eq };
	cb.call({ &view,1 });
	return true;
}

bool node::LuaStandaloneDiffEqnClass::HasBlockDialog() const
{
	return true;
}

static bool ValidateCanOpenEditor(node::model::FunctionalBlockData& data, node::logging::Logger& logger)
{
	if (data.properties.size() < 3)
	{
		logger.LogError("cannot open editor, expected 3 properties, found: {}", data.properties.size());
		return false;
	}
	if (data.properties[2].GetType() != node::model::BlockPropertyType::String)
	{
		logger.LogError("cannot open editor, expected string property, found: {}", data.properties[2].GetTypeAsString());
		return false;
	}
	return true;
}

std::unique_ptr<node::BlockDialog> node::LuaStandaloneDiffEqnClass::CreateBlockDialog(Scene& scene, std::shared_ptr<IBlockPropertiesUpdater> model_updater, model::BlockModel& model, model::FunctionalBlockData& data, std::any& simulation_data)
{
	UNUSED_PARAM(data);
	UNUSED_PARAM(simulation_data);
	if (!ValidateCanOpenEditor(data, m_logger))
	{
		return {};
	}

	return LuaEditorDialog::Create(model, std::move(model_updater), BlockClassPtr{BlockClassPtr::new_reference,this}, "Code", WidgetSize{500.0f,500.0f}, &scene);
}

tl::expected<std::monostate, std::string> node::LuaStandaloneDiffEqnClass::validate_lua_script(const std::string& code) const
{
	lua::DiffEqnBuilder builder{ m_logger };

	sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::math);
	auto expr_opt = builder.build_lua_functions(code, lua);
	if (!expr_opt)
	{
		return tl::unexpected<std::string>{std::move(expr_opt.error())};
	}

	return std::monostate{};
}
