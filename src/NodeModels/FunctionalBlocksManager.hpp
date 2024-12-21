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
	using property_t = std::variant<std::string, double, int64_t, uint64_t, bool>;
	BlockProperty(std::string name, BlockPropertyType type, property_t prop)
		:name{ name }, type{ type }, prop{ prop } {}
	std::string name;
	BlockPropertyType type;
	property_t prop;

	std::string to_string() const;
	static std::optional<property_t> from_string(BlockPropertyType type, std::string_view str);
	static std::optional<property_t> from_string(BlockPropertyType type, std::string&& str);
};

struct FunctionalBlockData
{
	std::string block_class;
	std::vector<BlockProperty> properties;
};

class FunctionalBlocksManager
{
public:
	FunctionalBlockData* GetDataForId(BlockId id);
	const FunctionalBlockData* GetDataForId(BlockId id) const;
	void SetDataForId(BlockId id, FunctionalBlockData&& data);
	bool EraseDataForId(BlockId id);
private:
	std::unordered_map<BlockId, FunctionalBlockData> m_data;
};

}

}