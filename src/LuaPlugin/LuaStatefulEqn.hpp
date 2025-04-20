#pragma once

#include "PluginAPI/BlockClass.hpp"
#include "PluginAPI/Logger.hpp"
#include "sol/sol.hpp"

namespace node
{

namespace lua
{

struct StatefulFunctions
{
	sol::protected_function apply;
	sol::protected_function update;
};

class NLStatefulEqnBuilder
{
public:
	NLStatefulEqnBuilder(logging::Logger log)
		:m_logger{log} { }

	tl::expected<StatefulFunctions, std::string> build_lua_functions(const std::string& code, sol::state& lua);
	tl::expected<std::string, std::string> ReadLuaFile(std::string_view file_name);
private:
	logging::Logger m_logger;
};

struct NLStatefulEqn : public opt::INLStatefulEquation
{
	NLStatefulEqn(sol::state lua, lua::StatefulFunctions funcs) : lua{ std::move(lua) }, funcs{ std::move(funcs) } {}

	opt::Status Apply(std::span<const double> input, std::span<double> output, double t, opt::NLStatefulEquationDataCRef data) override;
	opt::Status Update(std::span<const double> input, double t, opt::NLStatefulEquationDataRef data) override;
	const char* GetLastError() override
	{
		return last_error.c_str();
	}

	sol::state lua;
	lua::StatefulFunctions funcs;
	std::string last_error;
};

}
}