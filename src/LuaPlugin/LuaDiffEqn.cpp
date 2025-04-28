#include "LuaDiffEqn.hpp"
#include "sol/sol.hpp"
#include "PluginAPI/Logger.hpp"
#include <sstream>

tl::expected<node::lua::DiffFunctions, std::string> node::lua::DiffEqnBuilder::build_lua_functions(const std::string& code, sol::state& lua) const
{
	sol::protected_function_result result;

	try
	{
		result = lua.safe_script(code.c_str());
	}
	catch (std::exception& e)
	{
		return tl::unexpected<std::string>{std::format("exception creating Lua script: {}", std::string_view{ e.what() })};
	}

	if (!result.valid())
	{
		sol::error err = result;
		return tl::unexpected<std::string>{err.what()};
	}

	try
	{
		node::lua::DiffFunctions funcs;
		auto func = lua["apply"];
		if (!func.valid() || func.get_type() != sol::type::function)
		{
			return tl::unexpected<std::string>{"couldn't find 'apply' function"};
		}
		funcs.apply = std::move(func);
		return funcs;
	}
	catch (std::exception& e)
	{
		return tl::unexpected<std::string>{std::format("sol exception building script: {}", e.what())};
	}
}

tl::expected<std::monostate, std::string> node::lua::DiffEqnBuilder::AddUserTypes(sol::state& lua) const
{
	UNUSED_PARAM(lua);
	return std::monostate{};
}

opt::Status node::lua::DiffEqn::Apply(std::span<const double> input, std::span<double> output, const double t)
{
	auto call_result = funcs.apply(input, output, t);
	if (!call_result.valid())
	{
		sol::error err = call_result;
		last_error = err.what();
		return opt::Status::error;
	}
	return opt::Status::ok;
}
