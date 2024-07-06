#pragma once

#include <vector>

namespace node::model
{

template <typename EventArg>
class Publisher;

template <typename EventArg>
class Observer
{
	friend class Publisher<EventArg>;
public:
	virtual void OnEvent(EventArg& arg) = 0;
	virtual ~Observer()
	{
		for (auto&& publisher : m_publishers)
		{
			publisher->RemoveObserver(this);
		}
	}
	Observer() = default;
	Observer(const Observer&) = delete;
	Observer& operator=(const Observer&) = delete;
private:
	void AddPublisher(Publisher<EventArg>* publisher) 
	{ m_publishers.push_back(publisher); }
	void RemovePublisher(Publisher<EventArg>* publisher)
	{
		auto it = std::find(m_publishers.begin(), m_publishers.end(), publisher);
		if (it != m_publishers.end())
		{
			m_publishers.erase(it);
		}
	}
	std::vector<Publisher<EventArg>*> m_publishers;
};

template <typename EventArg>
class Publisher
{
	friend class Observer<EventArg>;
public:
	void Attach(Observer<EventArg>* observer) { 
		observer->AddPublisher(this);
		m_observers.push_back(observer); 
	}
	void Detach(Observer<EventArg>* observer) {
		auto it = std::find(m_observers.begin(), m_observers.end(), observer);
		if (it != m_observers.end())
		{
			observer->RemovePublisher(this);
			m_observers.erase(it);
		}
	}

	void Notify(EventArg& arg)
	{
		for (auto&& observer : m_observers)
		{
			observer->OnEvent(arg);
		}
	}

	Publisher() = default;
	virtual ~Publisher()
	{
		for (auto&& observer : m_observers)
		{
			observer->RemovePublisher(this);
		}
	}
	Publisher(const Publisher&) = delete;
	Publisher& operator=(const Publisher&) = delete;
private:
	void RemoveObserver(const Observer<EventArg>* observer)
	{
		auto it = std::find(m_observers.begin(), m_observers.end(), observer);
		if (it != m_observers.end())
		{
			m_observers.erase(it);
		}
	}
	std::vector<Observer<EventArg>*> m_observers;

};

}