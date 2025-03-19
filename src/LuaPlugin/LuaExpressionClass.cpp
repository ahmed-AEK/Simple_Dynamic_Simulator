#include "LuaExpressionClass.hpp"
#include "sol/sol.hpp"
#include <sstream>

static const std::vector<node::model::BlockProperty> ClassProperties{
	*node::model::BlockProperty::Create("Inputs Count", node::model::BlockPropertyType::UnsignedInteger, 0.0),
	*node::model::BlockProperty::Create("Expression", node::model::BlockPropertyType::String, "t"),
};

static constexpr std::string_view Description = "Parses Simple expressions using lua, inputs: a,b,c,d,e,f and t for time";

std::string_view node::LuaExpressionClass::GetName() const
{
	static constexpr std::string_view name = "LuaExpression";
	return name;
}

std::string_view node::LuaExpressionClass::GetDescription() const
{
	return Description;
}

std::span<const node::model::BlockProperty> node::LuaExpressionClass::GetDefaultClassProperties() const
{
	return ClassProperties;
}

namespace 
{

	std::optional<sol::protected_function> build_lua_expr(uint64_t in_sockets, std::string_view code, sol::state& lua)
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
			SDL_Log("exception creating Lua script: %s", e.what());
		}

		if (!result.valid())
		{
			return {};
		}

		try
		{
			auto func = lua["func"];
			if (func.get_type() != sol::type::function)
			{
				return {};
			}
			return sol::protected_function{ func };
		}
		catch (std::exception& e)
		{
			SDL_Log("sol exception building script: %s", e.what());
		}

		return {};
	}

	bool validate_properies_lua(uint64_t in_sockets, std::string_view code)
	{
		sol::state lua;
		lua.open_libraries(sol::lib::math);
		auto expr_opt = build_lua_expr(in_sockets, code, lua);
		if (!expr_opt)
		{
			return false;
		}

		sol::protected_function func = *expr_opt;
		std::vector<double> func_inputs(in_sockets + 1);
		auto call_result = func(sol::as_args(func_inputs));
		if (!call_result.valid())
		{
			return false;
		}
		if (call_result.return_count() != 1 || call_result.get_type(0) != sol::type::number)
		{
			return false;
		}
		try
		{
			[[maybe_unused]] double ret_value = call_result.get<double>(0);
		}
		catch (std::exception& e)
		{
			SDL_Log("sol exception building script return: %s",e.what());
			return false;
		}

		return true;
	}
}
bool node::LuaExpressionClass::ValidateClassProperties(const std::vector<model::BlockProperty>& properties) const
{
	if (properties.size() != ClassProperties.size())
	{
		return false;
	}
	for (size_t i = 0; i < properties.size(); i++)
	{
		if (properties[i].name != ClassProperties[i].name)
		{
			return false;
		}
		if (properties[i].GetType() != ClassProperties[i].GetType())
		{
			return false;
		}
		if (!(properties[i].prop.index() == ClassProperties[i].prop.index()))
		{
			return false;
		}
	}
	auto* in_sockets_count = properties[0].get_uint();
	assert(in_sockets_count);
	if (*in_sockets_count > 6)
	{
		return false;
	}
	auto* code = properties[1].get_str();
	assert(code);
	if (!code || !code->size())
	{
		return false;
	}
	return validate_properies_lua(*in_sockets_count,*code);
}

std::vector<node::model::SocketType> node::LuaExpressionClass::CalculateSockets(const std::vector<model::BlockProperty>& properties) const
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

node::BlockType node::LuaExpressionClass::GetBlockType(const std::vector<model::BlockProperty>& properties) const
{
	UNUSED_PARAM(properties);
	return BlockType::Stateful;
}

node::IBlockClass::GetFunctorResult node::LuaExpressionClass::GetFunctor(const std::vector<model::BlockProperty>& properties) const
{
	struct SolExprFunctorEqn : public opt::INLStatefulEquation
	{
		SolExprFunctorEqn(sol::state lua, sol::protected_function func) : lua{ std::move(lua) }, func{ std::move(func) } {}
		virtual void Apply(std::span<const double> input, std::span<double> output, double t, opt::NLStatefulEquationDataCRef data)
		{
			UNUSED_PARAM(data);
			auto call_result = func(sol::as_args(input), t);
			output[0] = call_result.get<double>(0);
		}
		virtual void Update(std::span<const double> input, double t, opt::NLStatefulEquationDataRef data)
		{
			UNUSED_PARAM(input);
			UNUSED_PARAM(t);
			UNUSED_PARAM(data);
		}
		sol::state lua;
		sol::protected_function func;
	};
	assert(ValidateClassProperties(properties));
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
	return opt::NLStatefulEquationWrapper{
		std::move(inputs),
		{static_cast<int32_t>(*in_sockets_count)},
		opt::make_NLStatefulEqn<SolExprFunctorEqn>(std::move(lua), std::move(*func)),
		{}
	};
}
