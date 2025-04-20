#include "LuaStatefulEqnClass.hpp"
#include "sol/sol.hpp"
#include "PluginAPI/Logger.hpp"
#include <sstream>

#include "LuaPlugin/LuaStatefulEqn.hpp"


static const std::vector<node::model::BlockProperty> ClassProperties{
	*node::model::BlockProperty::Create("Inputs Count", node::model::BlockPropertyType::UnsignedInteger, 0.0),
	*node::model::BlockProperty::Create("Path", node::model::BlockPropertyType::String, "DerivativeBlock.lua"),
};

static constexpr std::string_view Description = "Loads a lua file from Path that is a valid StatefulEqn";

void node::LuaStatefulEqnClass::GetName(GetNameCallback cb, void* context) const
{
	static constexpr std::string_view name = "LuaStatefulEqn";
	cb(context, name);
}

void node::LuaStatefulEqnClass::GetDescription(GetDescriptionCallback cb, void* context) const
{
	cb(context, Description);
}

void node::LuaStatefulEqnClass::GetDefaultClassProperties(GetDefaultClassPropertiesCallback cb, void* context) const
{
	cb(context, ClassProperties);
}

int node::LuaStatefulEqnClass::ValidateClassProperties(std::span<const model::BlockProperty> properties, IValidatePropertiesNotifier& error_cb) const
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
	auto* path = properties[1].get_str();
	assert(path);
	if (!path || !path->size())
	{
		error_cb.error(1, std::format("no path found"));
		return false;
	}
	auto validation_result = validate_lua_script(*path);
	if (!validation_result)
	{
		error_cb.error(1, validation_result.error());
		return false;
	}
	return true;
}

void node::LuaStatefulEqnClass::CalculateSockets(std::span<const model::BlockProperty> properties, CalculateSocketCallback cb, void* context) const
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

node::BlockType node::LuaStatefulEqnClass::GetBlockType(std::span<const model::BlockProperty> properties) const
{
	UNUSED_PARAM(properties);
	return BlockType::Stateful;
}

int node::LuaStatefulEqnClass::GetFunctor(std::span<const model::BlockProperty> properties, IGetFunctorCallback& cb) const
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
	auto* path = properties[1].get_str();

	lua::NLStatefulEqnBuilder builder{ m_logger };

	auto file_content = builder.ReadLuaFile(*path);
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

tl::expected<std::monostate, std::string> node::LuaStatefulEqnClass::validate_lua_script(std::string_view path) const
{
	lua::NLStatefulEqnBuilder builder{m_logger};

	auto file_content = builder.ReadLuaFile(path);
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
