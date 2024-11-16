#pragma once

#include <vector>
#include <memory_resource>
#include <functional>
#include <span>

namespace opt
{
class InputEquation
{
public:

	InputEquation(std::pmr::vector<int32_t> input_ids,
		std::pmr::memory_resource* resource = std::pmr::get_default_resource())
		:m_input_ids(std::move(input_ids)), 
		m_input_buffer(m_input_ids.size(), resource)
	{
	}
	[[nodiscard]] constexpr std::span<double> get_input_buffer() { return m_input_buffer; }
	[[nodiscard]] constexpr std::span<const int32_t> get_input_ids() const { return m_input_ids; }
	[[nodiscard]] constexpr std::span<int32_t> get_input_ids() { return m_input_ids; }
	[[nodiscard]] std::pmr::polymorphic_allocator<double> get_allocator() { return m_input_buffer.get_allocator(); }
protected:
	std::pmr::vector<int32_t> m_input_ids;
	std::pmr::vector<double> m_input_buffer;
};

class OutputEquation
{
public:

	OutputEquation(std::pmr::vector<int32_t> output_ids,
		std::pmr::memory_resource* resource = std::pmr::get_default_resource())
		:m_output_ids(std::move(output_ids)),
		m_output_buffer(m_output_ids.size(), resource)
	{
	}
	[[nodiscard]] constexpr std::span<double> get_output_buffer() { return m_output_buffer; }
	[[nodiscard]] constexpr std::span<const int32_t> get_output_ids() const { return m_output_ids; }
	[[nodiscard]] constexpr std::span<int32_t> get_output_ids() { return m_output_ids; }
	[[nodiscard]] std::pmr::polymorphic_allocator<double> get_allocator() { return m_output_buffer.get_allocator(); }
protected:
	std::pmr::vector<int32_t> m_output_ids;
	std::pmr::vector<double> m_output_buffer;
};

}