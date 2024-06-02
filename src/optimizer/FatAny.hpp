#pragma once
#include <array>
#include <typeinfo>
#include <typeindex>
#include <any>
#include <cassert>
#include <type_traits>
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
struct TypeManager
{
	static int marker;
};

template <typename T>
int TypeManager<T>::marker = 0;

template <typename T>
concept no_except_move_constructible = std::is_nothrow_move_constructible_v<T>;

class FatAny
{
	struct Empty_t {};
public:
	template <no_except_move_constructible T>
	explicit constexpr FatAny(T obj)
		: m_manager{
				+[](void* ptr) { static_cast<T*>(ptr)->~T(); },
				+[](void* src, void* dst) { new (dst) T(std::move(*static_cast<T*>(src)));} },
				p_manager{ &TypeManager<T>::marker }
	{
		static_assert(sizeof(T) <= sizeof(m_data.buff));
		m_manager.move_construct(&obj, m_data.buff.data());
	}
	constexpr FatAny() noexcept
		: m_manager{
			+[](void* ptr) { UNUSED_PARAM(ptr); },
			+[](void* src, void* dst) { UNUSED_PARAM(src); UNUSED_PARAM(dst); }
		},
		p_manager { nullptr } { };
	FatAny(FatAny&& other) noexcept;
	FatAny& operator=(FatAny&& other) noexcept;
	~FatAny();
	
	template <typename T>
	[[nodiscard]] constexpr bool contains() const
	{
		return p_manager == &TypeManager<T>::marker;
	}

	template <typename T>
	[[nodiscard]] constexpr T& get()
	{
		assert(p_manager == &TypeManager<T>::marker);
		return *reinterpret_cast<T*>(m_data.buff.data());
	}
	template <typename T>
	[[nodiscard]] constexpr const T& get() const
	{
		assert(p_manager == &TypeManager<T>::marker);
		return *reinterpret_cast<const T*>(m_data.buff.data());
	}

	[[nodiscard]] bool IsEmpty() const { return nullptr == p_manager; }

private:
	static constexpr size_t FatAnySize = std::max(2 * sizeof(void*), 2 * sizeof(double));
	union data_buffer {
		std::array<void*, FatAnySize / sizeof(void*)> buff;
	};
	data_buffer m_data;
	AnyManager m_manager;
	void* p_manager;
};
}