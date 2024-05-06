#pragma once
#include <array>
#include <typeinfo>
#include <typeindex>
#include <any>
#include <cassert>
#include <type_traits>
#include <cstddef>

namespace opt
{
struct AnyManager
{
	using destructor_t = void (*) (void*);
	using mover_t = void (*) (void*, void*);
	constexpr AnyManager(destructor_t destructor, mover_t mover) : destructor{ destructor }, mover{ mover } {}
	constexpr void move_construct(void* src, void* dst) noexcept { mover(src, dst); }
	constexpr void destruct(void* ptr) { destructor(ptr); }
private:
	destructor_t destructor = nullptr;
	mover_t mover = nullptr;
};
template <typename T>
concept no_except_move_constructible = std::is_nothrow_move_constructible_v<T>;

class FatAny
{
	struct Empty_t {};
public:
	template <no_except_move_constructible T>
	constexpr FatAny(T obj)
		: m_manager{
				+[](void* ptr) { static_cast<T*>(ptr)->~T(); },
				+[](void* src, void* dst) { new (dst) T(std::move(*static_cast<T*>(src)));} },
				m_contained{ std::type_index(typeid(T)) }
	{
		static_assert(sizeof(T) <= sizeof(m_data));
		m_manager.move_construct(&obj, m_data.data());
	}
	constexpr FatAny() noexcept : FatAny{ Empty_t{} } {};
	FatAny(FatAny&& other) noexcept;
	FatAny& operator=(FatAny&& other) noexcept;
	~FatAny();
	
	template <typename T>
	[[nodiscard]] constexpr bool contains() const
	{
		return m_contained == std::type_index(typeid(T));
	}

	template <typename T>
	[[nodiscard]] constexpr T& get()
	{
		assert(m_contained == std::type_index(typeid(T)));
		return *reinterpret_cast<T*>(m_data.data());
	}
	template <typename T>
	[[nodiscard]] constexpr const T& get() const
	{
		assert(m_contained == std::type_index(typeid(T)));
		return *reinterpret_cast<const T*>(m_data.data());
	}

	[[nodiscard]] bool IsEmpty() const { return std::type_index(typeid(Empty_t)) == m_contained; }

private:
	static constexpr size_t FatAnySize = std::max(2 * sizeof(void*), 2 * sizeof(double));
	std::array<char, FatAnySize> m_data;
	AnyManager m_manager;
	std::type_index m_contained;
};
}