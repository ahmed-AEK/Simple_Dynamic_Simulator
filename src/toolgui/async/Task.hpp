#pragma once

#include <coroutine>
#include <mutex>
#include <functional>
#include <condition_variable>

namespace node
{

namespace async
{


class Executor;

class TaskAwaitable;
class Timeout;
class ResumeParentAwaitable;

template <typename T>
class FutureAwaitable;
template <typename T>
class Future;

class Task
{
public:
	class promise
	{
	public:
		Task get_return_object() { return { std::coroutine_handle<promise>::from_promise(*this) }; }
		std::suspend_always initial_suspend() noexcept { return {}; }
		ResumeParentAwaitable final_suspend() noexcept;
		void unhandled_exception() 
		{
			m_error = std::current_exception();
		}
		void return_void() {}

		TaskAwaitable await_transform(Task&& task);
		Timeout await_transform(Timeout&& o);
		template <typename T>
		FutureAwaitable<T> await_transform(Future<T>&& o);

		void SetExecutor(Executor* executor)
		{
			m_executor = executor;
		}
		Executor* GetExecutor() const { return m_executor; }
		void SetCaller(std::coroutine_handle<promise> h) { m_caller = h; }
		std::coroutine_handle<promise> GetCaller() const { return m_caller; }
		std::exception_ptr get_error() const { return m_error; }
	private:
		std::exception_ptr m_error;
		Executor* m_executor;
		std::coroutine_handle<promise> m_caller;
	};

	using promise_type = promise;



	Task(Task&& other) noexcept
		: m_handle{ std::exchange(other.m_handle, std::coroutine_handle<promise>{}) }
	{
	}
	Task& operator=(Task&& other) noexcept
	{
		std::ranges::swap(m_handle, other.m_handle);
		return *this;
	}
	void Resume()
	{
		assert(m_handle);
		assert(!m_handle.done());
		m_handle.resume();
	}
	~Task()
	{
		if (m_handle)
		{
			m_handle.destroy();
		}
	}
	bool Done() const
	{
		assert(m_handle);
		if (!m_handle)
		{
			return true;
		}
		return m_handle.done();
	}
	bool Valid() const
	{
		return static_cast<bool>(m_handle);
	}
	std::coroutine_handle<promise_type> handle() const
	{
		return m_handle;
	}
	bool IsRoot() const
	{
		return !static_cast<bool>(m_handle.promise().GetCaller());
	}

	std::coroutine_handle<promise> Release()
	{
		return std::exchange(m_handle, std::coroutine_handle<promise>{});
	}
private:
	Task(std::coroutine_handle<promise_type> handle)
		:m_handle{ handle }
	{

	}
	friend promise;

	std::coroutine_handle<promise> m_handle;
};

class Executor
{
public:
	virtual void Dispatch(Task task) = 0;
	virtual void Post(Task task) = 0;
	virtual void MakeReady(std::coroutine_handle<Task::promise> h) = 0;
	virtual void MakeReadyDelayed(std::coroutine_handle<Task::promise> h, int delay) = 0;
	virtual void MakeReadyForDelete(std::coroutine_handle<Task::promise> h) = 0;
};


class TaskAwaitable
{
public:
	TaskAwaitable(Executor* executor, Task task)
		: executor{ executor }, task{ std::move(task) }
	{

	}

	bool await_ready() { return task.Done(); }
	std::coroutine_handle<Task::promise> await_suspend(std::coroutine_handle<Task::promise> h)
	{
		auto handle = task.handle();
		handle.promise().SetCaller(h);
		handle.promise().SetExecutor(executor);
		return handle;
	}
	void await_resume() {
		std::exception_ptr error;
		if (auto error_ptr = task.handle().promise().get_error())
		{
			error = std::move(error_ptr);
		}
		if (error)
		{
			std::rethrow_exception(error);
		}
	}

	Executor* executor;
	Task task;
};

class ResumeParentAwaitable
{
public:
	ResumeParentAwaitable(std::coroutine_handle<Task::promise> parent)
		: parent{ parent }
	{

	}

	bool await_ready() noexcept { return false; }
	std::coroutine_handle<> await_suspend(std::coroutine_handle<Task::promise> h) noexcept
	{
		if (parent)
		{
			return parent;
		}
		else if (auto* executor = h.promise().GetExecutor())
		{
			executor->MakeReadyForDelete(h);
		}
		return std::noop_coroutine();
	}
	void await_resume() noexcept {}

	std::coroutine_handle<Task::promise> parent;
};

inline TaskAwaitable Task::promise::await_transform(Task&& task)
{
	return TaskAwaitable{ m_executor, std::move(task) };
}

inline ResumeParentAwaitable Task::promise::final_suspend() noexcept {
	return ResumeParentAwaitable{m_caller};
}

class Timeout
{
public:
	Timeout(int32_t time)
		:m_timeout_period{time}
	{

	}

	bool await_ready() { return m_timeout_period == 0; }
	void await_suspend(std::coroutine_handle<Task::promise> h)
	{
		auto* executor = h.promise().GetExecutor();
		assert(executor);
		executor->MakeReadyDelayed(h, m_timeout_period);
	}
	void await_resume() {}
private:
	int32_t m_timeout_period;
};

inline Timeout Task::promise::await_transform(Timeout&& o) { return std::move(o); }

namespace detail
{
	template <typename T>
	struct SharedState
	{
		std::optional<T> object;
		std::mutex mutex;
		std::condition_variable cv;
		std::exception_ptr error;
		std::function<void()> on_completed;
		bool future_obtained = false;
	};
}

template <typename T>
class Future
{
public:

	Future(std::shared_ptr<detail::SharedState<T>> state)
		: m_state{state} { }
	Future(Future&&) = default;
	Future& operator=(Future&&) = default;

	void set_callback(std::function<void()> callback)
	{
		std::unique_lock l{ m_state->mutex };
		assert(!m_state->on_completed); // only 1 callback allowed
		if (!m_state->object && !m_state->error)
		{
			m_state->on_completed = std::move(callback);
		}
		else
		{
			callback();
		}
	}
	T get()
	{
		T result = [&]() {
			std::unique_lock l{ m_state->mutex };
			m_state->cv.wait(l, [&]() ->bool { return m_state->object.has_value() || m_state->error; });
			if (m_state->error)
			{
				std::rethrow_exception(m_state->error);
			}
			return std::move(*m_state->object);
			}();
		m_state = nullptr;
		return result;
	}
	bool ready()
	{
		std::unique_lock l{ m_state->mutex };
		return m_state->object || m_state->error ;
	}
private:
	std::shared_ptr<detail::SharedState<T>> m_state;
};

template <typename T>
class ThreadedPromise
{
public:
	ThreadedPromise()
		: m_state{ std::make_shared<detail::SharedState<T>>() }
	{

	}
	void set_value(T value)
	{
		std::unique_lock l{ m_state->mutex };
		assert(!m_state->object && !m_state->error);
		m_state->object = std::move(value);
		if (m_state->on_completed)
		{
			m_state->on_completed();
		}
		m_state->cv.notify_all();
	}
	void set_error(std::exception_ptr error)
	{
		std::unique_lock l{ m_state->mutex };
		assert(!m_state->object && !m_state->error);
		m_state->error = std::move(error);
		if (m_state->on_completed)
		{
			m_state->on_completed();
		}
		m_state->cv.notify_all();
	}
	Future<T> get_future()
	{
		assert(!m_state->future_obtained);
		m_state->future_obtained = true;
		return Future<T>(m_state);
	}

private:
	std::shared_ptr<detail::SharedState<T>> m_state;
};

template <typename T>
class FutureAwaitable
{
public:
	FutureAwaitable(Future<T> fut)
		:m_future{ std::move(fut) }
	{
	}

	bool await_ready() noexcept { return m_future.ready(); }
	void await_suspend(std::coroutine_handle<Task::promise> h)
	{
		assert(h.promise().GetExecutor());
		m_future.set_callback([exec = h.promise().GetExecutor(), handle = h]() {
				exec->MakeReady(handle);
			});
	}
	T await_resume() {
		return m_future.get();
	}
private:
	Future<T> m_future;
};
template <typename T>
FutureAwaitable<T> Task::promise::await_transform(Future<T>&& o)
{
	return FutureAwaitable{ std::move(o) };
}
}

using Task = async::Task;

}