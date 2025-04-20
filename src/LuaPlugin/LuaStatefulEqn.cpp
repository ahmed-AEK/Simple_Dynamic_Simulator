#include "LuaStatefulEqn.hpp"
#include "sol/sol.hpp"
#include "PluginAPI/Logger.hpp"
#include <sstream>

tl::expected<node::lua::StatefulFunctions, std::string> node::lua::NLStatefulEqnBuilder::build_lua_functions(const std::string& code, sol::state& lua)
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
		node::lua::StatefulFunctions funcs;
		auto func = lua["apply"];
		if (!func.valid() || func.get_type() != sol::type::function)
		{
			return tl::unexpected<std::string>{"couldn't find 'apply' function"};
		}
		funcs.apply = std::move(func);
		auto update_func = lua["update"];
		if (!update_func.valid() || update_func.get_type() != sol::type::function)
		{
			return tl::unexpected<std::string>{"couldn't find 'update' function"};
		}
		funcs.update = std::move(update_func);
		return funcs;
	}
	catch (std::exception& e)
	{
		return tl::unexpected<std::string>{std::format("sol exception building script: {}", e.what())};
	}
}

tl::expected<std::string, std::string> node::lua::NLStatefulEqnBuilder::ReadLuaFile(std::string_view file_name)
{
	std::string path_str{ file_name };
	auto* current_dir = SDL_GetCurrentDirectory();
	std::string full_path = std::string{ current_dir } + "LuaScripts/" + path_str;
	SDL_free(current_dir);
	SDL_IOStream* file_stream = SDL_IOFromFile(full_path.c_str(), "r");
	if (!file_stream)
	{
		return tl::unexpected<std::string>{std::format("Failed to open file: {} , reason: {}", path_str, SDL_GetError())};
	}
	std::string file_content;
	std::array<char, 1024> temp_buffer;
	while (auto bytes_read = SDL_ReadIO(file_stream, temp_buffer.data(), 1024))
	{
		if (file_content.size() > 1024 * 1024 * 8)
		{
			return tl::unexpected<std::string>{std::format("File too big!: {}", path_str)};
		}
		file_content.insert(file_content.end(), temp_buffer.data(), temp_buffer.data() + bytes_read);
	}
	SDL_CloseIO(file_stream);
	file_stream = nullptr;
	node::logger(node::logging::LogCategory::Extension)
		.LogDebug("Read Lua file bytes total = %d", static_cast<int>(file_content.size()));

	return file_content;
}

opt::Status node::lua::NLStatefulEqn::Apply(std::span<const double> input, std::span<double> output, double t, opt::NLStatefulEquationDataCRef data)
{
	UNUSED_PARAM(data);
	auto call_result = funcs.apply(input, output, t, data);
	if (!call_result.valid())
	{
		sol::error err = call_result;
		last_error = err.what();
		return opt::Status::error;
	}
	return opt::Status::ok;
}

opt::Status node::lua::NLStatefulEqn::Update(std::span<const double> input, double t, opt::NLStatefulEquationDataRef data)
{
	UNUSED_PARAM(input);
	UNUSED_PARAM(t);
	UNUSED_PARAM(data);
	auto call_result = funcs.update(input, t, data);
	if (!call_result.valid())
	{
		sol::error err = call_result;
		last_error = err.what();
		return opt::Status::error;
	}
	return opt::Status::ok;
}