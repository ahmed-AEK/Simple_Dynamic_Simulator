#pragma once
#include <cassert>
#include <vector>

namespace node::model
{

template <typename EventType>
class Publisher;

template <typename EventType>
class SinglePublisher;

template <typename EventType>
class MultiPublisher;

template <typename EventType>
class SingleObserver;

template <typename EventType>
class MultiObserver;

template <typename EventType>
class Observer
{
    friend SinglePublisher<EventType>;
    friend MultiPublisher<EventType>;
public:
    virtual void OnNotify(EventType&) = 0;
    Observer() noexcept {}
    // virtual destructor
    virtual ~Observer() = default;
    Observer(const Observer&) = delete;
    Observer& operator=(const Observer&) = delete;
private:
    virtual void RemovePublisher(Publisher<EventType>&) = 0;
    virtual void AddPublisher(Publisher<EventType>&) = 0;
};

template <typename EventType>
class Publisher
{
    friend SingleObserver<EventType>;
    friend MultiObserver<EventType>;
public:
    Publisher() noexcept {}
    virtual void Notify(EventType&) = 0;
    virtual void Attach(Observer<EventType>&) = 0;
    virtual void Detach(Observer<EventType>&) = 0;

    // virtual destructor
    virtual ~Publisher() = default;
    Publisher(const Publisher&) = delete;
    Publisher& operator=(const Publisher&) = delete;
private:
    virtual void DetachDestruct(Observer<EventType>& observer) = 0;

};

template <typename EventType>
class SinglePublisher : public Publisher<EventType>
{
public:
    SinglePublisher() noexcept {}
    void Notify(EventType& e) override
    {
        if (m_observer)
        {
            m_observer->OnNotify(e);
        }
    }
    void Attach(Observer<EventType>& observer) override
    {
        assert(m_observer == nullptr);
        if (!m_observer)
        {
            m_observer = &observer;
            m_observer->AddPublisher(*this);
        }
    }
    void Detach(Observer<EventType>& observer) override
    {
        if (m_observer == &observer)
        {
            m_observer = nullptr;
            observer.RemovePublisher(*this);
        }
    }
    ~SinglePublisher() {
        if (m_observer)
        {
            m_observer->RemovePublisher(*this);
        }
    }
private:
    void DetachDestruct(Observer<EventType>& observer) override
    {
        if (m_observer == &observer)
        {
            m_observer = nullptr;
        }
    }
    Observer<EventType>* m_observer = nullptr;
};

template <typename EventType>
class MultiPublisher : public Publisher<EventType>
{
public:
    MultiPublisher() noexcept {}
    void Notify(EventType& e) override
    {
        for (auto&& observer : m_observers)
        {
            observer->OnNotify(e);
        }
    }

    void Attach(Observer<EventType>& observer) override
    {
        m_observers.push_back(&observer);
        observer.AddPublisher(*this);
    }
    void Detach(Observer<EventType>& observer) override
    {
        auto it = std::find(m_observers.begin(), m_observers.end(), &observer);
        assert(it != m_observers.end());
        if (it != m_observers.end())
        {
            (*it)->RemovePublisher(*this);
            m_observers.erase(it);
        }
    }
    ~MultiPublisher() {
        for (auto&& observer : m_observers)
        {
            observer->RemovePublisher(*this);
        }
    }
private:
    void DetachDestruct(Observer<EventType>& observer) override
    {
        auto it = std::find(m_observers.begin(), m_observers.end(), &observer);
        assert(it != m_observers.end());
        if (it != m_observers.end())
        {
            m_observers.erase(it);
        }
    }
    std::vector<Observer<EventType>*> m_observers;
};

template <typename EventType>
class SingleObserver : public Observer<EventType>
{
public:
    SingleObserver() noexcept {}

    ~SingleObserver() {
        if (m_publisher)
        {
            m_publisher->DetachDestruct(*this);
        }
    }
private:
    void RemovePublisher(Publisher<EventType>& publisher) override
    {
        assert(m_publisher == &publisher);
        if (&publisher == m_publisher)
        {
            m_publisher = nullptr;
        }
    }
    virtual void AddPublisher(Publisher<EventType>& publisher)
    {
        assert(m_publisher == nullptr);
        m_publisher = &publisher;
    }

    Publisher<EventType>* m_publisher = nullptr;
};

template <typename EventType>
class MultiObserver : public Observer<EventType>
{
public:
    MultiObserver() noexcept {}

    ~MultiObserver() {
        for (auto&& publisher : m_publishers)
        {
            publisher->DetachDestruct(*this);
        }
    }
private:
    void RemovePublisher(Publisher<EventType>& publisher) override
    {
        auto it = std::find(m_publishers.begin(), m_publishers.end(), &publisher);
        assert(it != m_publishers.end());
        if (it != m_publishers.end())
        {
            m_publishers.erase(it);
        }
    }
    virtual void AddPublisher(Publisher<EventType>& publisher)
    {
        m_publishers.push_back(&publisher);
    }

    std::vector<Publisher<EventType>*> m_publishers;
};

}