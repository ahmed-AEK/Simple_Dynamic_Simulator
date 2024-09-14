#include "BlockModel.hpp"
#include <cassert>
#include <algorithm>
#include <charconv>

// helper type for the visitor #4
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;


std::string node::model::BlockProperty::to_string() const
{
	return std::visit(overloaded{
				[](const std::string& prop) -> std::string {return prop; },
				[](const bool& prop)->std::string { return prop ? "1" : "0"; },
				[](const auto& prop) ->std::string {
					char buffer[25];
					auto [ptr, ec] = std::to_chars(std::begin(buffer), std::end(buffer), prop);
					if (ec == std::errc{})
					{
						return std::string(std::string_view{ buffer, ptr });
					}
					return std::string{};
				} },
		prop);
}

std::optional<std::reference_wrapper<node::model::BlockSocketModel>> 
node::model::BlockModel::GetSocketById(SocketId id)
{
	auto vec = GetSockets();
	auto iter = std::find_if(vec.begin(), vec.end(),
		[id](const BlockSocketModel& socket) {return id == socket.GetId(); });
	if (iter != vec.end())
	{
		return { *iter };
	}
	return std::nullopt;
}

std::optional<std::reference_wrapper<const node::model::BlockSocketModel>> 
node::model::BlockModel::GetSocketById(SocketId id) const
{
	auto vec = GetSockets();
	auto iter = std::find_if(vec.begin(), vec.end(),
		[id](const BlockSocketModel& socket) {return id == socket.GetId(); });
	if (iter != vec.end())
	{
		return { *iter };
	}
	return std::nullopt;
}

void node::model::BlockModel::SetId(const BlockId& id)
{
	m_Id = id;
}

