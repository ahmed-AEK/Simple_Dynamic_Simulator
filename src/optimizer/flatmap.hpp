#pragma once

#include <vector>
#include <optional>
#include <span>
#include <string>
#include <cassert>

namespace opt
{

class FlatMap
{
public:
	explicit constexpr FlatMap(int32_t size = 0) : m_data(size) {};

	constexpr void modify(const int32_t& index, const double& item)
	{
		m_data[index] = item;
	}
	[[nodiscard]] constexpr double get(const int32_t& key) const
	{
		assert(static_cast<size_t>(key) < m_data.size());
		return m_data[key];
	}
	[[nodiscard]] constexpr std::span<double> data()
	{
		return m_data;
	}
	[[nodiscard]] constexpr int32_t size() const { return static_cast<int32_t>(m_data.size()); }
	void copy_to(FlatMap& other)
	{
		assert(size() == other.size());
		for (size_t i = 0; i < m_data.size(); i++)
		{
			other.m_data[i] = m_data[i];
		}
	}
	constexpr static void sync(FlatMap& src, FlatMap& dest)
	{
		if (src.size() == dest.size())
		{
			src.copy_to(dest);
		}
		else
		{
			dest = src;
		}
	}
	constexpr void clear()
	{
		m_data.clear();
	}
private:
	std::vector<double> m_data;
};
}