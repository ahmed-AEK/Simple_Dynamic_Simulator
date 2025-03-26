#include "LuaStatefulEqnClass.hpp"
#include "sol/sol.hpp"
#include <sstream>

static const std::vector<node::model::BlockProperty> ClassProperties{
	*node::model::BlockProperty::Create("Inputs Count", node::model::BlockPropertyType::UnsignedInteger, 0.0),
	*node::model::BlockProperty::Create("Path", node::model::BlockPropertyType::String, "DerivativeBlock.lua"),
};

static constexpr std::string_view Description = "Parses Simple expressions using lua, inputs: a,b,c,d,e,f and t for time";

std::string_view node::LuaStatefulEqnClass::GetName() const
{
	static constexpr std::string_view name = "LuaStatefulEqn";
	return name;
}

std::string_view node::LuaStatefulEqnClass::GetDescription() const
{
	return Description;
}

std::span<const node::model::BlockProperty> node::LuaStatefulEqnClass::GetDefaultClassProperties() const
{
	return ClassProperties;
}

struct SolNLFunctions
{
	std::optional<sol::protected_function> apply;
	std::optional<sol::protected_function> update;
};

namespace
{

	std::optional<SolNLFunctions> build_lua_expr(uint64_t in_sockets, const std::string& code, sol::state& lua)
	{
		UNUSED_PARAM(in_sockets);
		sol::protected_function_result result;

		try
		{
			result = lua.safe_script(code.c_str());
		}
		catch (std::exception& e)
		{
			SDL_Log("exception creating Lua script: %s", e.what());
		}

		if (!result.valid())
		{
			return {};
		}

		try
		{
			SolNLFunctions funcs;
			auto func = lua["apply"];
			if (func.valid() && func.get_type() == sol::type::function)
			{
				funcs.apply = std::move(func);
			}
			auto update_func = lua["update"];
			if (update_func.valid() && update_func.get_type() == sol::type::function)
			{
				funcs.update = std::move(update_func);
			}
			return funcs;
		}
		catch (std::exception& e)
		{
			SDL_Log("sol exception building script: %s", e.what());
		}

		return {};
	}

	std::optional<std::string> ReadLuaFile(std::string_view path)
	{
		std::string path_str{ path };
		auto* current_dir = SDL_GetCurrentDirectory();
		std::string full_path = std::string{ current_dir } + "LuaScripts/" + path_str;
		SDL_free(current_dir);
		SDL_IOStream* file_stream = SDL_IOFromFile(full_path.c_str(), "r");
		if (!file_stream)
		{
			SDL_Log("Failed to open file: %s , reason: %s", path_str.c_str(), SDL_GetError());
			return {};
		}
		std::string file_content;
		std::array<char, 1024> temp_buffer;
		while (auto bytes_read = SDL_ReadIO(file_stream, temp_buffer.data(), 1024))
		{
			if (file_content.size() > 1024 * 1024 * 8)
			{
				SDL_Log("File too big!: %s", path_str.c_str());
				return {};
			}
			file_content.insert(file_content.end(), temp_buffer.data(), temp_buffer.data() + bytes_read);
		}
		SDL_CloseIO(file_stream);
		file_stream = nullptr;
		SDL_Log("Read Lua file bytes total = %d", static_cast<int>(file_content.size()));

		return file_content;
	}
	bool validate_properies_lua(uint64_t in_sockets, std::string_view path)
	{
		auto file_content = ReadLuaFile(path);
		if (!file_content)
		{
			return false;
		}
		sol::state lua;
		lua.open_libraries(sol::lib::base, sol::lib::math);
		auto expr_opt = build_lua_expr(in_sockets, *file_content, lua);
		if (!expr_opt)
		{
			return false;
		}

		return true;
	}
}
bool node::LuaStatefulEqnClass::ValidateClassProperties(const std::vector<model::BlockProperty>& properties, IValidatePropertiesNotifier& error_cb) const
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
	auto validation_result = validate_properies_lua(*in_sockets_count, *path);
	if (!validation_result)
	{
		error_cb.error(1, std::format("script load failed!"));
		return false;
	}
	return true;
}

std::vector<node::model::SocketType> node::LuaStatefulEqnClass::CalculateSockets(const std::vector<model::BlockProperty>& properties) const
{
	using model::SocketType;
	if (properties.size() < 2)
	{
		assert(false);
		return {};
	}

	auto* in_sockets_count = properties[0].get_uint();
	if (!in_sockets_count)
	{
		assert(false);
		return {};
	}

	std::vector<SocketType> sockets;
	sockets.reserve(1 + *in_sockets_count);
	for (size_t i = 0; i < *in_sockets_count; i++)
	{
		sockets.push_back(SocketType::input);
	}
	sockets.push_back(SocketType::output);
	return sockets;
}

node::BlockType node::LuaStatefulEqnClass::GetBlockType(const std::vector<model::BlockProperty>& properties) const
{
	UNUSED_PARAM(properties);
	return BlockType::Stateful;
}

node::IBlockClass::GetFunctorResult node::LuaStatefulEqnClass::GetFunctor(const std::vector<model::BlockProperty>& properties) const
{
	struct SolNLFunctorEqn : public opt::INLStatefulEquation
	{
		SolNLFunctorEqn(sol::state lua, SolNLFunctions funcs) : lua{ std::move(lua) }, funcs{ std::move(funcs) } {}
		opt::Status Apply(std::span<const double> input, std::span<double> output, double t, opt::NLStatefulEquationDataCRef data) override
		{
			UNUSED_PARAM(data);
			auto call_result = (*funcs.apply)(input, output, t, data);
			if (!call_result.valid())
			{
				sol::error err = call_result;
				last_error = err.what();
				return opt::Status::error;
			}
			return opt::Status::ok;
		}
		opt::Status Update(std::span<const double> input, double t, opt::NLStatefulEquationDataRef data) override
		{
			UNUSED_PARAM(input);
			UNUSED_PARAM(t);
			UNUSED_PARAM(data);
			auto call_result = (*funcs.update)(input, t, data);
			if (!call_result.valid())
			{
				sol::error err = call_result;
				last_error = err.what();
				return opt::Status::error;
			}
			return opt::Status::ok;
		}
		const char* GetLastError() override
		{
			return last_error.c_str();
		}

		sol::state lua;
		SolNLFunctions funcs;
		std::string last_error;
	};
	
	[[maybe_unused]] LightValidatePropertiesNotifier notifier;
	auto valid = ValidateClassProperties(properties, notifier);
	if (notifier.errored || !valid)
	{
		return std::string{ "failed to validate properties" };
	}

	auto* in_sockets_count = properties[0].get_uint();
	assert(in_sockets_count);
	sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::math);
	auto* path = properties[1].get_str();
	auto file_content = ReadLuaFile(*path);
	assert(file_content);
	auto funcs = build_lua_expr(*in_sockets_count, *file_content, lua);
	if (!funcs)
	{
		return std::string{ "failed to parse lua file" };
	}
	if (!funcs->apply)
	{
		return std::string{ "missing Apply function" };
	}
	if (!funcs->update)
	{
		return std::string{ "missing Update function" };
	}

	std::vector<int32_t> inputs;
	inputs.reserve(*in_sockets_count);
	for (size_t i = 0; i < *in_sockets_count; i++)
	{
		inputs.push_back(static_cast<int32_t>(i));
	}
	return opt::NLStatefulEquationWrapper{
		std::move(inputs),
		{static_cast<int32_t>(*in_sockets_count)},
		opt::make_NLStatefulEqn<SolNLFunctorEqn>(std::move(lua), std::move(*funcs)),
		{}
	};
}
