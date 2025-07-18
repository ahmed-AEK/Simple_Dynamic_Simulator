#pragma once

#include "toolgui/async/Task.hpp"
#include <stack>
#include <thread>

#include "PluginAPI/Logger.hpp"

namespace node
{
namespace async
{

bool make_task_ready_threadsafe(std::coroutine_handle<Task::promise> h);

class SDLLoop: public async::Executor
{
public:
	void Dispatch(Task task) override;
	void Post(Task task) override;
	void MakeReady(std::coroutine_handle<Task::promise> h) override;
	void MakeReadyDelayed(std::coroutine_handle<Task::promise> h, int32_t delay) override;
	void MakeReadyForDelete(std::coroutine_handle<Task::promise> h) override;

	void SetThreadAffinity(std::thread::id thread_id = std::this_thread::get_id());

	void DispatchReadyTasks();

private:
	std::vector<Task> m_tasks;
	std::stack<std::coroutine_handle<Task::promise>> m_ready_tasks;
	std::stack<std::coroutine_handle<Task::promise>> m_ended_tasks;
	std::thread::id m_thread_id;
	logging::Logger m_logger = logger(logging::LogCategory::Core);
};


}


}
