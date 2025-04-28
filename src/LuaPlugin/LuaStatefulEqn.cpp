#include "LuaStatefulEqn.hpp"
#include "sol/sol.hpp"
#include "PluginAPI/Logger.hpp"
#include <sstream>

tl::expected<node::lua::StatefulFunctions, std::string> node::lua::NLStatefulEqnBuilder::build_lua_functions(const std::string& code, sol::state& lua) const
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
		auto setup_func = lua["setup"];
		if (setup_func.valid() && setup_func.get_type() == sol::type::function)
		{
			funcs.setup = std::move(setup_func);
		}
		funcs.update = std::move(update_func);
		return funcs;
	}
	catch (std::exception& e)
	{
		return tl::unexpected<std::string>{std::format("sol exception building script: {}", e.what())};
	}
}

tl::expected<std::monostate, std::string> node::lua::NLStatefulEqnBuilder::AddUserTypes(sol::state& lua) const
{
	using opt::StatefulEquationEvent;

	lua.new_usertype<StatefulEquationEvent>("StatefulEquationEvent",
		"enabled", &StatefulEquationEvent::enabled,
		"set", &StatefulEquationEvent::set,
		"t", &StatefulEquationEvent::t
	);

	lua.new_usertype<LuaNLStatefulEquationDataCRef>("NLStatefulEquationDataCRef",
		"crossings", sol::readonly(&LuaNLStatefulEquationDataCRef::crossings),
		"ev", sol::readonly(&LuaNLStatefulEquationDataCRef::ev)
	);

	lua.new_usertype<LuaNLStatefulEquationDataRef>("NLStatefulEquationDataRef",
		"crossings", sol::readonly(&LuaNLStatefulEquationDataRef::crossings),
		"ev", sol::readonly(&LuaNLStatefulEquationDataRef::ev)
	);

	using opt::ZeroCrossDescriptor;

	lua.new_enum("CrossType", 
		"rising", ZeroCrossDescriptor::CrossType::rising,
		"falling", ZeroCrossDescriptor::CrossType::falling,
		"both", ZeroCrossDescriptor::CrossType::both
		);

	lua.new_enum("CrossPosition",
		"undefined", ZeroCrossDescriptor::Position::undefined,
		"above", ZeroCrossDescriptor::Position::above,
		"below", ZeroCrossDescriptor::Position::below
		);

	lua.new_usertype<ZeroCrossDescriptor>("ZeroCrossDescriptor",
		"value", &ZeroCrossDescriptor::value,
		"in_port_id", &ZeroCrossDescriptor::in_port_id,
		"type", &ZeroCrossDescriptor::type,
		"last_value", &ZeroCrossDescriptor::last_value,
		"current_value", &ZeroCrossDescriptor::current_value
	);

	using opt::NLStatefulEquationData;
	lua.new_usertype<NLStatefulEquationData>("NLStatefulEquationData",
		"crossings", sol::readonly(&NLStatefulEquationData::crossings),
		"ev", sol::readonly(&NLStatefulEquationData::ev)
		);
	return std::monostate{};
}


opt::Status node::lua::NLStatefulEqn::Setup(opt::NLStatefulEquationData& data)
{
	if (!funcs.setup)
	{
		return opt::Status::ok;
	}
	auto call_result = (*funcs.setup)(data);
	if (!call_result.valid())
	{
		sol::error err = call_result;
		last_error = err.what();
		return opt::Status::error;
	}
	return opt::Status::ok;
}

opt::Status node::lua::NLStatefulEqn::Apply(std::span<const double> input, std::span<double> output, double t, opt::NLStatefulEquationDataCRef data)
{
	LuaNLStatefulEquationDataCRef data_lua{ &data.crossings, &data.ev };
	auto call_result = funcs.apply(input, output, t, data_lua);
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