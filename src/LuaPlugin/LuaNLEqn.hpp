#pragma once

#include "PluginAPI/BlockClass.hpp"
#include "PluginAPI/Logger.hpp"
#include "sol/sol.hpp"

namespace node
{

namespace lua
{

struct NLFunctions
{
	sol::protected_function apply;
};

class NLEqnBuilder
{
public:
	NLEqnBuilder(logging::Logger log = logger(logging::LogCategory::Extension))
		:m_logger{log} { }

	tl::expected<NLFunctions, std::string> build_lua_functions(const std::string& code, sol::state& lua) const;
	tl::expected<std::monostate, std::string> AddUserTypes(sol::state& lua) const;
private:
	logging::Logger m_logger;
};

struct NLEqn : public opt::INLEquation
{
	NLEqn(sol::state lua, lua::NLFunctions funcs) : lua{ std::move(lua) }, funcs{ std::move(funcs) } {}

	opt::Status Apply(std::span<const double> input, std::span<double> output) override;
	const char* GetLastError() override
	{
		return last_error.c_str();
	}

	sol::state lua;
	lua::NLFunctions funcs;
	std::string last_error;
};

}
}