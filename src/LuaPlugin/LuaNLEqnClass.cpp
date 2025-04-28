#include "LuaNLEqnClass.hpp"
#include "sol/sol.hpp"
#include "PluginAPI/Logger.hpp"
#include <sstream>

#include "LuaPlugin/Utility.hpp"
#include "LuaPlugin/LuaNLEqn.hpp"


static const std::vector<node::model::BlockProperty> ClassProperties{
	*node::model::BlockProperty::Create("Inputs Count", node::model::BlockPropertyType::UnsignedInteger, 2),
	*node::model::BlockProperty::Create("Outputs Count", node::model::BlockPropertyType::UnsignedInteger, 1),
	*node::model::BlockProperty::Create("Path", node::model::BlockPropertyType::String, "MultiplyBlock.lua"),
};

static constexpr std::string_view Description = "Loads a lua file from Path that is a valid NLEqn";

void node::LuaNLEqnClass::GetName(GetNameCallback cb, void* context) const
{
	static constexpr std::string_view name = "LuaNLEqn";
	cb(context, name);
}

void node::LuaNLEqnClass::GetDescription(GetDescriptionCallback cb, void* context) const
{
	cb(context, Description);
}

void node::LuaNLEqnClass::GetDefaultClassProperties(GetDefaultClassPropertiesCallback cb, void* context) const
{
	cb(context, ClassProperties);
}

int node::LuaNLEqnClass::ValidateClassProperties(std::span<const model::BlockProperty> properties, IValidatePropertiesNotifier& error_cb) const
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

void node::LuaNLEqnClass::CalculateSockets(std::span<const model::BlockProperty> properties, CalculateSocketCallback cb, void* context) const
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

node::BlockType node::LuaNLEqnClass::GetBlockType(std::span<const model::BlockProperty> properties) const
{
	UNUSED_PARAM(properties);
	return BlockType::Stateless;
}

int node::LuaNLEqnClass::GetFunctor(std::span<const model::BlockProperty> properties, IGetFunctorCallback& cb) const
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
	auto* path = properties[2].get_str();

	lua::NLEqnBuilder builder{ m_logger };
	builder.AddUserTypes(lua);

	auto file_content = lua::ReadLuaFile(*path);
	assert(file_content);
	auto funcs = builder.build_lua_functions(*file_content, lua);
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
	opt::NLEquationWrapper eq{
		std::move(inputs),
		std::move(outputs),
		opt::make_NLEqn<lua::NLEqn>(std::move(lua), std::move(*funcs)),
	};

	node::BlockView view{ eq };
	cb.call({ &view,1 });
	return true;
}

tl::expected<std::monostate, std::string> node::LuaNLEqnClass::validate_lua_script(std::string_view path) const
{
	lua::NLEqnBuilder builder{m_logger};

	auto file_content = lua::ReadLuaFile(path);
	if (!file_content)
	{
		return tl::unexpected<std::string>{std::move(file_content.error())};
	}
	sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::math);
	auto expr_opt = builder.build_lua_functions(*file_content, lua);
	if (!expr_opt)
	{
		return tl::unexpected<std::string>{std::move(expr_opt.error())};
	}

	return std::monostate{};
}
