#pragma once
#include <utility>

template <typename T>
void incremet_reference(const T& o)
    requires requires { o.increment_ref(); }
{
    o.increment_ref();
}
template <typename T>
void decremet_reference(const T& o)
    requires requires { o.decrement_ref(); }
{
    o.decrement_ref();
}

template <class T>
class IntrusivePtr
{
public:
    struct new_reference_t {};
    struct steal_reference_t {};
    static constexpr new_reference_t new_reference{};
    static constexpr steal_reference_t steal_reference{};

    constexpr IntrusivePtr() = default;
    constexpr IntrusivePtr(nullptr_t) :IntrusivePtr{} {};
    constexpr IntrusivePtr& operator=(nullptr_t) {
        if (m_ptr)
        {
            incremet_reference(*m_ptr);
        }
        m_ptr = nullptr;
        return *this;
    }
    constexpr IntrusivePtr(new_reference_t, T* ptr)
        :m_ptr{ ptr } {
        if (m_ptr) { incremet_reference(*m_ptr); }
    }
    constexpr IntrusivePtr(steal_reference_t, T* ptr)
        : m_ptr{ ptr } {
    }

    constexpr T* get() const { return m_ptr; }
    constexpr T* release() { return std::exchange(m_ptr, nullptr); }
    constexpr ~IntrusivePtr() {
        if (m_ptr) { decremet_reference(*m_ptr); }
    }

    constexpr T& operator*() const { return *m_ptr; }
    constexpr T* operator->() const { return m_ptr; }

    constexpr IntrusivePtr(const IntrusivePtr& other)
        :m_ptr{ other.m_ptr }
    {
        if (m_ptr) { incremet_reference(*m_ptr); }
    }
    constexpr IntrusivePtr& operator=(const IntrusivePtr& other)
    {
        if (this == &other) { return *this; }
        if (m_ptr) { m_ptr->decrement_ref(); }
        m_ptr = other.m_ptr;
        if (m_ptr) { incremet_reference(*m_ptr); }
        return *this;
    }

    constexpr IntrusivePtr(IntrusivePtr&& other) noexcept
        :m_ptr{ std::exchange(other.m_ptr, nullptr) }
    {
    }
    constexpr IntrusivePtr& operator=(IntrusivePtr&& other) noexcept
    {
        if (this == &other) { return *this; }
        if (m_ptr) { decremet_reference(*m_ptr); }
        m_ptr = std::exchange(other.m_ptr, nullptr);
        return *this;
    }

    constexpr friend bool operator==(const IntrusivePtr& lhs, const IntrusivePtr& rhs) { return lhs.m_ptr == rhs.m_ptr; }

    constexpr friend bool operator==(const IntrusivePtr& lhs, nullptr_t) { return lhs.get() == nullptr; }
    constexpr friend bool operator==(nullptr_t, const IntrusivePtr& rhs) { return nullptr == rhs.get(); }

    constexpr operator bool() const { return m_ptr; }
    constexpr operator T* () const { return m_ptr; }
private:
    T* m_ptr = nullptr;
};

template <typename T, typename...Ts>
IntrusivePtr<T> make_intrusive(Ts&& ... args)
{
    return IntrusivePtr<T>{IntrusivePtr<T>::steal_reference, new T{ std::forward<Ts>(args)... }};
}

