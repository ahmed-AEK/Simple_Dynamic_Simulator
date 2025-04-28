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
	std::optional<sol::protected_function> setup;
};

struct LuaNLStatefulEquationDataCRef
{
	std::span<const opt::ZeroCrossDescriptor>* crossings;
	const opt::StatefulEquationEvent* ev;
};

struct LuaNLStatefulEquationDataRef
{
	std::span<opt::ZeroCrossDescriptor>* crossings;
	opt::StatefulEquationEvent* ev;
};

class NLStatefulEqnBuilder
{
public:
	NLStatefulEqnBuilder(logging::Logger log)
		:m_logger{log} { }

	tl::expected<StatefulFunctions, std::string> build_lua_functions(const std::string& code, sol::state& lua) const;
	tl::expected<std::monostate, std::string> AddUserTypes(sol::state& lua) const;
private:
	logging::Logger m_logger;
};

struct NLStatefulEqn : public opt::INLStatefulEquation
{
	NLStatefulEqn(sol::state lua, lua::StatefulFunctions funcs) : lua{ std::move(lua) }, funcs{ std::move(funcs) } {}

	opt::Status Setup(opt::NLStatefulEquationData& data);
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