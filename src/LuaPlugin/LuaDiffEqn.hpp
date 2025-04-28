#pragma once

#include "PluginAPI/BlockClass.hpp"
#include "PluginAPI/Logger.hpp"
#include "sol/sol.hpp"

namespace node
{

namespace lua
{

struct DiffFunctions
{
	sol::protected_function apply;
};

class DiffEqnBuilder
{
public:
	DiffEqnBuilder(logging::Logger log = logger(logging::LogCategory::Extension))
		:m_logger{log} { }

	tl::expected<DiffFunctions, std::string> build_lua_functions(const std::string& code, sol::state& lua) const;
	tl::expected<std::monostate, std::string> AddUserTypes(sol::state& lua) const;
private:
	logging::Logger m_logger;
};

struct DiffEqn : public opt::IDiffEquation
{
	DiffEqn(sol::state lua, lua::DiffFunctions funcs) : lua{ std::move(lua) }, funcs{ std::move(funcs) } {}

	opt::Status Apply(std::span<const double> input, std::span<double> output, const double t) override;
	const char* GetLastError() override
	{
		return last_error.c_str();
	}

	sol::state lua;
	lua::DiffFunctions funcs;
	std::string last_error;
};

}
}