#include "LuaExpressionClass.hpp"
#include "sol/sol.hpp"
#include <sstream>
#include "PluginAPI/Logger.hpp"

static const std::vector<node::model::BlockProperty> ClassProperties{
	*node::model::BlockProperty::Create("Inputs Count", node::model::BlockPropertyType::UnsignedInteger, 0.0),
	*node::model::BlockProperty::Create("Expression", node::model::BlockPropertyType::String, "t"),
};

static constexpr std::string_view Description = "Parses Simple expressions using lua, inputs: a,b,c,d,e,f and t for time";

void node::LuaExpressionClass::GetName(GetNameCallback cb, void* context) const
{
	static constexpr std::string_view name = "LuaExpression";
	cb(context,name);
}

void node::LuaExpressionClass::GetDescription(GetDescriptionCallback cb, void* context) const
{
	cb(context, Description);
}

void node::LuaExpressionClass::GetDefaultClassProperties(GetDefaultClassPropertiesCallback cb, void* context) const
{
	cb(context, ClassProperties);
}

namespace 
{

	tl::expected<sol::protected_function, std::string> build_lua_expr(uint64_t in_sockets, std::string_view code, sol::state& lua)
	{
		assert(in_sockets < 7);
		static constexpr std::array<char, 6> arg_mapping{ 'a','b','c','d','e','f' };
		std::ostringstream function_def;
		function_def << "function func (";
		for (size_t i = 0; i < in_sockets; i++)
		{
			function_def << arg_mapping[i] << ",";
		}
		function_def << "t)\n return " << code << "\n end";

		sol::protected_function_result result;
		try
		{
			result = lua.safe_script(function_def.str());
		}
		catch (std::exception& e)
		{
			return tl::unexpected<std::string>{std::format("exception creating Lua script: {}", e.what())};
		}

		if (!result.valid())
		{
			sol::error err = result;
			return tl::unexpected<std::string>{err.what()};
		}

		try
		{
			auto func = lua["func"];
			if (func.get_type() != sol::type::function)
			{
				return tl::unexpected<std::string>{std::string_view{ "unknown internal error in lua!"}};
			}
			return sol::protected_function{ func };
		}
		catch (std::exception& e)
		{
			return tl::unexpected<std::string>{std::format("sol exception building script: {}", e.what())};
		}
	}

	tl::expected<std::monostate, std::string> validate_properies_lua(uint64_t in_sockets, std::string_view code)
	{
		sol::state lua;
		lua.open_libraries(sol::lib::math);
		auto expr_opt = build_lua_expr(in_sockets, code, lua);
		if (!expr_opt)
		{
			return tl::unexpected<std::string>{std::move(expr_opt.error())};
		}

		sol::protected_function func = *expr_opt;
		std::vector<double> func_inputs(in_sockets + 1);
		auto call_result = func(sol::as_args(func_inputs));
		if (!call_result.valid())
		{
			sol::error err = call_result;
			return tl::unexpected<std::string>{err.what()};
		}
		if (call_result.return_count() != 1 || call_result.get_type(0) != sol::type::number)
		{
			auto type = call_result.get_type(0);
			if (type == sol::type::nil)
			{
				return tl::unexpected<std::string>{"expression contains undefined value, only t,a,b,c,d,e,f are allowed!"};
			}
			return tl::unexpected<std::string>{std::format("expression did not result in a number!, expression type id: {}", static_cast<int>(type))};
		}
		try
		{
			[[maybe_unused]] double ret_value = call_result.get<double>(0);
		}
		catch (std::exception& e)
		{
			return tl::unexpected<std::string>{std::format("sol exception building script return: {}", e.what())};
		}

		return std::monostate{};
	}
}
int node::LuaExpressionClass::ValidateClassProperties(std::span<const model::BlockProperty> properties, IValidatePropertiesNotifier& error_cb) const
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
	auto* code = properties[1].get_str();
	assert(code);
	if (!code || !code->size())
	{
		return false;
	}
	auto validation_result = validate_properies_lua(*in_sockets_count,*code);
	if (!validation_result)
	{
		error_cb.error(1, validation_result.error());
		return false;
	}
	return true;
}

void node::LuaExpressionClass::CalculateSockets(std::span<const model::BlockProperty> properties, CalculateSocketCallback cb, void* context) const
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

node::BlockType node::LuaExpressionClass::GetBlockType(std::span<const model::BlockProperty> properties) const
{
	UNUSED_PARAM(properties);
	return BlockType::Stateful;
}

int node::LuaExpressionClass::GetFunctor(std::span<const model::BlockProperty> properties, IGetFunctorCallback& cb) const
{
	struct SolExprFunctorEqn : public opt::INLStatefulEquation
	{
		SolExprFunctorEqn(sol::state lua, sol::protected_function func) : lua{ std::move(lua) }, func{ std::move(func) } {}
		opt::Status Apply(std::span<const double> input, std::span<double> output, double t, opt::NLStatefulEquationDataCRef data) override
		{
			UNUSED_PARAM(data);
			auto call_result = func(sol::as_args(input), t);
			if (!call_result.valid())
			{
				sol::error err = call_result;
				last_error = err.what();
				return opt::Status::error;
			}
			output[0] = call_result.get<double>(0);
			return opt::Status::ok;
		}
		opt::Status Update(std::span<const double> input, double t, opt::NLStatefulEquationDataRef data) override
		{
			UNUSED_PARAM(input);
			UNUSED_PARAM(t);
			UNUSED_PARAM(data);
			return opt::Status::ok;
		}
		const char* GetLastError() override
		{ return last_error.c_str(); }

		sol::state lua;
		sol::protected_function func;
		std::string last_error;
	};

	[[maybe_unused]] LightValidatePropertiesNotifier notifier;
	auto valid = ValidateClassProperties(properties, notifier);
	if (notifier.errored || !valid)
	{
		cb.error("failed to validate properties");
		return false;
	}
	auto* in_sockets_count = properties[0].get_uint();
	sol::state lua;
	lua.open_libraries(sol::lib::math);
	auto* code = properties[1].get_str();
	auto func = build_lua_expr(*in_sockets_count, *code, lua);
	std::vector<int32_t> inputs;
	inputs.reserve(*in_sockets_count);
	for (size_t i = 0; i < *in_sockets_count; i++)
	{
		inputs.push_back(static_cast<int32_t>(i));
	}
	opt::NLStatefulEquationWrapper eq{
		std::move(inputs),
		{static_cast<int32_t>(*in_sockets_count)},
		opt::make_NLStatefulEqn<SolExprFunctorEqn>(std::move(lua), std::move(*func)),
		{}
	};
	node::BlockView view{ eq };
	cb.call({ &view,1 });
	return true;
}
