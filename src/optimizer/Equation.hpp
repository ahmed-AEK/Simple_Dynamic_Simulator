#pragma once

#include <vector>
#include <memory_resource>
#include <functional>
#include <span>

namespace opt
{
class Equation
{
public:

	Equation(std::pmr::vector<int64_t> input_ids,
		std::pmr::vector<int64_t> output_ids,
		std::pmr::memory_resource* resource = std::pmr::get_default_resource())
		:m_input_ids(std::move(input_ids)), m_output_ids(std::move(output_ids)),
		m_input_buffer(m_input_ids.size(), resource), m_output_buffer(m_output_ids.size(), resource)
	{
	}
	[[nodiscard]] constexpr std::span<double> get_input_buffer() { return m_input_buffer; }
	[[nodiscard]] constexpr std::span<double> get_output_buffer() { return m_output_buffer; }
	[[nodiscard]] constexpr std::span<const int64_t> get_input_ids() const { return m_input_ids; }
	[[nodiscard]] constexpr std::span<const int64_t> get_output_ids() const { return m_output_ids; }
	[[nodiscard]] std::pmr::polymorphic_allocator<double> get_allocator() { return m_output_buffer.get_allocator(); }
protected:
	std::pmr::vector<int64_t> m_input_ids;
	std::pmr::vector<int64_t> m_output_ids;
	std::pmr::vector<double> m_input_buffer;
	std::pmr::vector<double> m_output_buffer;
};
}