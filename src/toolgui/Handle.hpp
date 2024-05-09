#pragma once
#include <utility>
#include <atomic>
#include <memory_resource>
#include <memory>

namespace node
{
template <typename T>
struct Handle
{
    std::atomic<T*> object;
    bool isAlive() const {return object.load() != nullptr;}
    void Destroy() { object.store(nullptr);}
    T* getObject() { return object.load(); }
    const T* getObject() const { return object.load(); }
};

template <typename T>
class HandlePtr
{
    std::shared_ptr<Handle<T>> m_ptr;
public:
    HandlePtr() = default;
    HandlePtr(std::shared_ptr<Handle<T>> ptr) : m_ptr(std::move(ptr)) {}
    T* GetObjectPtr() { if (m_ptr) { return m_ptr->object;} else { return nullptr;}}
    const T* GetObjectPtr() const { if (m_ptr) { return m_ptr->object; } else { return nullptr; } }
    bool isAlive() const {if (m_ptr && m_ptr->isAlive()) {return true;} else { return false; }}
};

template <typename T>
class HandleOwnigPtr
{
    std::shared_ptr<Handle<T>> m_ptr;
public:
    HandleOwnigPtr(std::shared_ptr<Handle<T>> ptr): m_ptr{ptr} {}
    HandleOwnigPtr(const HandleOwnigPtr<T>&) = delete;
    HandleOwnigPtr& operator=(const HandleOwnigPtr<T>&) = delete;
    HandleOwnigPtr(HandleOwnigPtr<T>&&) = delete;
    HandleOwnigPtr& operator=(HandleOwnigPtr<T>&&) = delete;
    ~HandleOwnigPtr(){ m_ptr->Destroy();}
    std::shared_ptr<Handle<T>> Reset() { return std::exchange(m_ptr, nullptr); };
    void Destroy() { m_ptr->object.Destroy(); }
    HandlePtr<T> GetHandlePtr() { return { m_ptr }; }
    const HandlePtr<T> GetHandlePtr() const { return { m_ptr }; }
};

template <typename T>
class HandleAllocator
{
public:
    static HandleOwnigPtr<T> CreateHandle(T* ptr);
public:
    constexpr static size_t initial_buffer_size = 1024;
};

template <typename T>
HandleOwnigPtr<T> HandleAllocator<T>::CreateHandle(T *ptr)
{
    static char initial_buffer[HandleAllocator<T>::initial_buffer_size];

    static std::pmr::monotonic_buffer_resource monotonicResource(
        initial_buffer,
        sizeof(initial_buffer)
    );

    static std::pmr::unsynchronized_pool_resource poolResource{
        std::pmr::pool_options{0, sizeof(Handle<T>) + 40},
        &monotonicResource
    };

    std::pmr::polymorphic_allocator<Handle<T>> alloc(&poolResource);
    std::shared_ptr<Handle<T>> obj = std::allocate_shared<Handle<T>>(alloc, ptr);
    return obj;
}


}