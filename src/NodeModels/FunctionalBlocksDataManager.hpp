#pragma once

#include "NodeModels/IdTypes.hpp"

#include <unordered_map>
#include <string>
#include <variant>
#include <optional>

namespace node
{
namespace model
{

enum class BlockPropertyType
{
	String,
	FloatNumber, // float
	Integer, // maybe negative
	UnsignedInteger, // unsigned
	Boolean,
};

struct BlockProperty
{

	struct TypeGetter
	{
		BlockPropertyType operator()(const std::string&) const { return BlockPropertyType::String; }
		BlockPropertyType operator()(const double&) const { return BlockPropertyType::FloatNumber; }
		BlockPropertyType operator()(const int64_t&) const { return BlockPropertyType::Integer; }
		BlockPropertyType operator()(const uint64_t&) const { return BlockPropertyType::UnsignedInteger; }
		BlockPropertyType operator()(const bool&) const { return BlockPropertyType::Boolean; }
	};

	struct TypeGetterString
	{
		std::string_view operator()(const std::string&) const { return "String"; }
		std::string_view operator()(const double&) const { return "FloatNumber"; }
		std::string_view operator()(const int64_t&) const { return "Integer"; }
		std::string_view operator()(const uint64_t&) const { return "UnsignedInteger"; }
		std::string_view operator()(const bool&) const { return "Boolean"; }
	};
	using property_t = std::variant<std::string, double, int64_t, uint64_t, bool>;
	BlockProperty(std::string name, property_t prop)
		:name{ name }, prop{ prop } {}
	static std::optional<BlockProperty> Create(std::string name, BlockPropertyType prop_type, property_t prop);

	std::string name;
	property_t prop;
	
	BlockPropertyType GetType() const { return std::visit(TypeGetter{}, prop); }
	std::string_view GetTypeAsString() const { return std::visit(TypeGetterString{}, prop); }

	std::string to_string() const;
	static std::optional<property_t> from_string(BlockPropertyType type, std::string_view str);
	static std::optional<property_t> from_string(BlockPropertyType type, std::string&& str);

	const uint64_t* get_uint() const { return std::get_if<uint64_t>(&prop); }

	const int64_t* get_int() const { return std::get_if<int64_t>(&prop); }

	const double* get_float() const { return std::get_if<double>(&prop); }

	const bool* get_bool() const { return std::get_if<bool>(&prop); }

	const std::string* get_str() const { return std::get_if<std::string>(&prop); }
};

struct FunctionalBlockData
{
	std::string block_class;
	std::vector<BlockProperty> properties;
};

template <typename T>
class BlocksDataManager
{
public:
	T* GetDataForId(BlockId id)
	{
		auto it = m_data.find(id);
		if (it != m_data.end())
		{
			return &(it->second);
		}
		return nullptr;
	}

	const T* GetDataForId(BlockId id) const
	{
		auto it = m_data.find(id);
		if (it != m_data.end())
		{
			return &(it->second);
		}
		return nullptr;
	}

	void SetDataForId(BlockId id, T&& data)
	{
		m_data[id] = std::move(data);
	}

	bool EraseDataForId(BlockId id)
	{
		auto it = m_data.find(id);
		if (it != m_data.end())
		{
			m_data.erase(it);
			return true;
		}
		return false;
	}

	const std::unordered_map<BlockId, T>& GetData() const
	{
		return m_data;
	}

private:
	std::unordered_map<BlockId, T> m_data;
};

using FunctionalBlocksDataManager = BlocksDataManager<FunctionalBlockData>;

}

}