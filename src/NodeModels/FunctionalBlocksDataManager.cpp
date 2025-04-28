#include "FunctionalBlocksDataManager.hpp"
#include <boost/charconv.hpp>

// helper type for the visitor #4
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };


std::optional<node::model::BlockProperty> node::model::BlockProperty::Create(std::string name, BlockPropertyType prop_type, property_t prop)
{
	/*
	String:
	FloatNumber, // float
	Integer, // maybe negative
	UnsignedInteger, // unsigned
	Boolean,

	std::string, double, int64_t, uint64_t, bool
	*/
	switch (prop_type)
	{
		using enum BlockPropertyType;
	case String:
	{
		if (std::get_if<std::string>(&prop)) { return BlockProperty{ std::move(name), std::move(prop) }; }
		break;
	}
	case FloatNumber: // float
	{
		if (std::get_if<double>(&prop)) { return BlockProperty{ std::move(name), prop }; }
		if (auto* ptr = std::get_if<int64_t>(&prop)) { return BlockProperty{ std::move(name), static_cast<double>(*ptr) }; }
		if (auto* ptr = std::get_if<uint64_t>(&prop)) { return BlockProperty{ std::move(name), static_cast<double>(*ptr)}; }
		if (auto* ptr = std::get_if<bool>(&prop)) { return BlockProperty{ std::move(name), static_cast<double>(*ptr) }; }
		break;
	}
	case Integer: // maybe negative
	{
		if (std::get_if<int64_t>(&prop)) { return BlockProperty{ std::move(name), prop }; }
		if (auto* ptr = std::get_if<double>(&prop)) { return BlockProperty{ std::move(name), static_cast<int64_t>(*ptr) }; }
		if (auto* ptr = std::get_if<uint64_t>(&prop)) { return BlockProperty{ std::move(name), static_cast<int64_t>(*ptr) }; }
		if (auto* ptr = std::get_if<bool>(&prop)) { return BlockProperty{ std::move(name), static_cast<int64_t>(*ptr) }; }
		break;
	}
	case UnsignedInteger: // unsigned
	{
		if (std::get_if<uint64_t>(&prop)) { return BlockProperty{ std::move(name), prop }; }
		if (auto* ptr = std::get_if<double>(&prop)) { return BlockProperty{ std::move(name), static_cast<uint64_t>(*ptr) }; }
		if (auto* ptr = std::get_if<int64_t>(&prop)) { return BlockProperty{ std::move(name), static_cast<uint64_t>(*ptr) }; }
		if (auto* ptr = std::get_if<bool>(&prop)) { return BlockProperty{ std::move(name), static_cast<uint64_t>(*ptr) }; }
		break;
	}
	case Boolean:
	{
		if (std::get_if<bool>(&prop)) { return BlockProperty{ std::move(name), prop }; }
		if (auto* ptr = std::get_if<double>(&prop)) { return BlockProperty{ std::move(name), static_cast<bool>(*ptr) }; }
		if (auto* ptr = std::get_if<int64_t>(&prop)) { return BlockProperty{ std::move(name), static_cast<bool>(*ptr) }; }
		if (auto* ptr = std::get_if<uint64_t>(&prop)) { return BlockProperty{ std::move(name), static_cast<bool>(*ptr) }; }
		break;
	}
	}
	assert(false);
	return std::nullopt;
}

std::optional<node::model::BlockProperty::property_t> node::model::BlockProperty::from_string(BlockPropertyType type, std::string_view str)
{
	switch (type)
	{
		using enum model::BlockPropertyType;
	case Integer:
	{
		int64_t new_value = 0;
		auto res = boost::charconv::from_chars(str.data(), str.data() + str.size(), new_value);
		if (res.ec != std::errc{} || res.ptr != str.data() + str.size())
		{
			return std::nullopt;
		}
		return new_value;
	}
	case UnsignedInteger:
	{
		uint64_t new_value = 0;
		auto res = boost::charconv::from_chars(str.data(), str.data() + str.size(), new_value);
		if (res.ec != std::errc{} || res.ptr != str.data() + str.size())
		{
			return std::nullopt;
		}
		return new_value;
	}
	case FloatNumber:
	{
		double new_value = 0;
		auto res = boost::charconv::from_chars(str.data(), str.data() + str.size(), new_value);
		if (res.ec != std::errc{} || res.ptr != str.data() + str.size())
		{
			return std::nullopt;
		}
		return new_value;
	}
	case Boolean:
	{
		int64_t new_value = 0;
		auto res = boost::charconv::from_chars(str.data(), str.data() + str.size(), new_value);
		if (res.ec != std::errc{} || res.ptr != str.data() + str.size())
		{
			return std::nullopt;
		}
		return static_cast<bool>(new_value);
	}
	case String:
	{
		return std::string{ str };
	}
	}
	return std::nullopt;
}

std::optional<node::model::BlockProperty::property_t> node::model::BlockProperty::from_string(BlockPropertyType type, std::string&& str)
{
	if (type == BlockPropertyType::String)
	{
		return std::move(str);
	}
	return from_string(type, std::string_view{ str });
}

std::string node::model::BlockProperty::to_string() const
{
	return std::visit(overloaded{
				[](const std::string& prop) -> std::string {return prop; },
				[](const bool& prop)->std::string { return prop ? "1" : "0"; },
				[](const auto& prop) ->std::string {
					char buffer[25];
					auto [ptr, ec] = boost::charconv::to_chars(std::begin(buffer), std::end(buffer), prop);
					if (ec == std::errc{})
					{
						return std::string(std::string_view{ buffer, ptr });
					}
					return std::string{};
				} },
		prop);
}
