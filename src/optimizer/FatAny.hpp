#pragma once
#include <array>
#include <cassert>
#include <type_traits>
#include <new>
#include <cstddef>
#include "toolgui/NodeMacros.h"

namespace opt
{
struct AnyManager
{
	using destructor_t = void (*) (void*);
	using mover_t = void (*) (void*, void*);
	constexpr AnyManager(destructor_t destructor, mover_t mover) : destructor{ destructor }, mover{ mover } {}
	constexpr void move_construct(void* src, void* dst) const noexcept { mover(src, dst); }
	constexpr void destruct(void* ptr) const { destructor(ptr); }
private:
	destructor_t destructor = nullptr;
	mover_t mover = nullptr;
};

template <typename T>
struct SpecificManager;

template <typename T>
struct SpecificManager: public AnyManager
{
	constexpr SpecificManager()
		:AnyManager{ 
			+[](void* ptr) { static_cast<T*>(ptr)->~T(); },
			+[](void* src, void* dst) { new (dst) T(std::move(*static_cast<T*>(src))); } 
		}
	{
	}
	SpecificManager(const SpecificManager&) = delete;
	SpecificManager& operator=(const SpecificManager&) = delete;
};

template <typename T>
inline constexpr SpecificManager<T> specificManager{};

template <typename T>
concept no_except_move_constructible = std::is_nothrow_move_constructible_v<T>;

class FatAny
{
public:
	template <no_except_move_constructible T>
	explicit constexpr FatAny(T obj)
		: p_manager{&specificManager<T>}
	{
		static_assert(sizeof(T) <= sizeof(m_data.buff));
		p_manager->move_construct(&obj, m_data.buff.data());
	}
	constexpr FatAny() noexcept
		: p_manager{nullptr} { };
	FatAny(FatAny&& other) noexcept;
	FatAny& operator=(FatAny&& other) noexcept;
	FatAny(const FatAny&) = delete;
	FatAny& operator=(const FatAny&) = delete;
	~FatAny();
	
	template <typename T>
	[[nodiscard]] constexpr bool contains() const
	{
		return p_manager == &specificManager<std::decay_t<T>>;
	}

	template <typename T>
	[[nodiscard]] constexpr T& get()
	{
		assert(p_manager == &specificManager<std::decay_t<T>>);
		return *std::launder(reinterpret_cast<T*>(m_data.buff.data()));
	}
	template <typename T>
	[[nodiscard]] constexpr const T& get() const
	{
		assert(p_manager == &specificManager<std::decay_t<T>>);
		return *std::launder(reinterpret_cast<const T*>(m_data.buff.data()));
	}

	[[nodiscard]] bool IsEmpty() const { return nullptr == p_manager; }

private:
	static constexpr size_t FatAnySize = std::max(4 * sizeof(void*), 4 * sizeof(double));
	union data_buffer {
		alignas(FatAnySize) std::array<void*, FatAnySize / sizeof(void*)> buff;
	};
	data_buffer m_data;
	const AnyManager* p_manager;
};
}