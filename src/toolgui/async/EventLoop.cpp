#include "async/EventLoop.hpp"


static void LogTaskError(const std::coroutine_handle<node::Task::promise>& task, const node::logging::Logger& logger)
{
	std::exception_ptr error = task.promise().get_error();
	if (error)
	{
		try
		{
			std::rethrow_exception(error);
		}
		catch (const std::exception& e)
		{
			logger.LogError("error thrown in coroutine: {}", e.what());
		}
		catch (...)
		{
			logger.LogError("unknown error thrown in coroutine");
		}
	}
}

void node::async::SDLLoop::DispatchReadyTasks()
{
	while (m_ready_tasks.size())
	{
		auto top_task = std::move(m_ready_tasks.top());
		m_ready_tasks.pop();
		assert(top_task.promise().GetExecutor() == this);
		top_task.resume();
		if (m_ended_tasks.size() && m_ended_tasks.top() == top_task)
		{
			m_ended_tasks.pop();
			auto it_task = std::find_if(m_tasks.begin(), m_tasks.end(), [&](const Task& t) { return t.handle() == top_task; });
			assert(it_task != m_tasks.end());
			LogTaskError(it_task->handle(), m_logger);
			m_tasks.erase(it_task);
		}
	}

	while (m_ended_tasks.size())
	{
		auto top_task = std::move(m_ended_tasks.top());
		m_ended_tasks.pop();
		auto it_task = std::find_if(m_tasks.begin(), m_tasks.end(), [&](const Task& t) { return t.handle() == top_task; });
		assert(it_task != m_tasks.end());
		LogTaskError(it_task->handle(), m_logger);
		m_tasks.erase(it_task);
	}
}

void node::async::SDLLoop::SetThreadAffinity(std::thread::id thread_id)
{
	m_thread_id = std::move(thread_id);
}

void node::async::SDLLoop::Dispatch(Task task)
{
	assert(m_thread_id == std::this_thread::get_id());
	m_tasks.push_back(std::move(task));
	auto handle = m_tasks.back().handle();
	handle.promise().SetExecutor(this);

	handle.resume();
	if (m_ended_tasks.size() && m_ended_tasks.top() == handle)
	{
		m_ended_tasks.pop();
		auto it = std::find_if(m_tasks.cbegin(), m_tasks.cend(), [&](const Task& t) { return t.handle() == handle; });
		assert(it != m_tasks.cend());
		LogTaskError(it->handle(), m_logger);
		m_tasks.erase(it);
	}
}

void node::async::SDLLoop::Post(Task task)
{
	assert(m_thread_id == std::this_thread::get_id());
	m_tasks.push_back(std::move(task));
	auto handle = m_tasks.back().handle();
	handle.promise().SetExecutor(this);
	m_ready_tasks.push(handle);
}
void node::async::SDLLoop::MakeReady(std::coroutine_handle<Task::promise> h)
{
	if (m_thread_id == std::this_thread::get_id())
	{
		m_ready_tasks.push(h);
	}
	else
	{
		make_task_ready_threadsafe(h);
	}
}

void node::async::SDLLoop::MakeReadyForDelete(std::coroutine_handle<Task::promise> h)
{
	assert(m_thread_id == std::this_thread::get_id());
	m_ended_tasks.push(h);
}

bool node::async::make_task_ready_threadsafe(std::coroutine_handle<Task::promise> h)
{
	SDL_Event event{};
	event.type = SDL_EVENT_USER;
	event.user.type = SDL_EVENT_USER;
	event.user.timestamp = SDL_GetTicks();
	event.user.code = 2;
	event.user.data1 = h.address();
	return SDL_PushEvent(&event);
}

namespace {
	uint32_t timer_callback(void* userdata, SDL_TimerID, Uint32)
	{
		[[maybe_unused]] bool pushed = 
			make_task_ready_threadsafe(
				std::coroutine_handle<node::async::Task::promise>::from_address(userdata));
		assert(pushed);
		return 0;
	}
}

void node::async::SDLLoop::MakeReadyDelayed(std::coroutine_handle<Task::promise> h, int32_t delay)
{
	SDL_AddTimer(delay, timer_callback, h.address());
}
