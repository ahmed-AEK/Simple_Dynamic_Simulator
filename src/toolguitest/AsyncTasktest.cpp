#include "gtest/gtest.h"
#include "toolgui/async/Task.hpp"

using namespace node;

namespace 
{

Task test_task(int* value)
{
	*value = 5;
	co_return;
}

}

TEST(testAsyncTask, testResume)
{
	int value = 1;
	Task task = test_task(&value);

	EXPECT_EQ(value, 1);
	EXPECT_FALSE(task.Done());

	task.Resume();
	EXPECT_EQ(value, 5);
	EXPECT_TRUE(task.Valid());
	EXPECT_TRUE(task.Done());
}

namespace
{

Task test_task_timeout(int* value)
{
	*value = 1;
	co_await async::Timeout{ 5 };
	*value = 2;
	co_return;
}

struct TestExecutor : public async::Executor
{
	void Dispatch(Task task) override
	{
		m_tasks.push_back(std::move(task));
		m_tasks.back().handle().promise().SetExecutor(this);
		m_tasks.back().Resume();
	}
	void Post(Task task) override
	{
		m_tasks.push_back(std::move(task));
		m_tasks.back().handle().promise().SetExecutor(this);
		made_ready.push_back(m_tasks.back().handle());
	}
	void MakeReady(std::coroutine_handle<Task::promise> h) override
	{
		made_ready.push_back(h);
	}
	void MakeReadyDelayed(std::coroutine_handle<Task::promise> h, int32_t delay) override
	{
		made_ready.push_back(h);
		passed_delay = delay;
	}
	virtual void MakeReadyForDelete(std::coroutine_handle<Task::promise> h)
	{
		made_ready_for_delete.push_back(h);
	}


	std::optional<int32_t> passed_delay;
	std::vector<Task> m_tasks;
	std::vector<std::coroutine_handle<Task::promise>> made_ready;
	std::vector<std::coroutine_handle<Task::promise>> made_ready_for_delete;

	TestExecutor() = default;
	TestExecutor& operator=(TestExecutor&&) = default;
	TestExecutor(TestExecutor&&) = default;
	~TestExecutor()
	{
	}
};

}

TEST(testAsyncTask, testSleep)
{
	TestExecutor executor;
	int wait_value = 0;
	Task task = test_task_timeout(&wait_value);

	EXPECT_EQ(wait_value, 0);

	executor.Dispatch(std::move(task));
	EXPECT_EQ(wait_value, 1);
	ASSERT_EQ(executor.m_tasks.size(), 1);
	EXPECT_TRUE(executor.passed_delay);
	EXPECT_EQ(*executor.passed_delay, 5);

	executor.m_tasks[0].Resume();
	EXPECT_EQ(wait_value, 2);
	ASSERT_EQ(executor.m_tasks.size(), 1);
	EXPECT_TRUE(executor.m_tasks[0].Done());
	EXPECT_EQ(executor.made_ready_for_delete.size(), 1);

}

namespace
{

Task test_wait_only_task()
{
	co_await async::Timeout{ 3 };
}

Task test_task_await_task(int* value)
{
	*value = 1;
	co_await test_wait_only_task();
	*value = 2;
	co_return;
}
}

TEST(testAsyncTask, testAwaitAnotherTask)
{
	TestExecutor executor;
	int wait_value = 0;

	Task task = test_task_await_task(&wait_value);

	EXPECT_EQ(wait_value, 0);

	executor.Dispatch(std::move(task));
	EXPECT_EQ(wait_value, 1);

	ASSERT_EQ(executor.m_tasks.size(), 1);
	EXPECT_TRUE(executor.passed_delay);
	EXPECT_EQ(*executor.passed_delay, 3);
	ASSERT_EQ(executor.made_ready.size(), 1);

	executor.made_ready[0].resume();

	EXPECT_EQ(wait_value, 2);
	ASSERT_EQ(executor.m_tasks.size(), 1);
	EXPECT_TRUE(executor.m_tasks[0].Done());
	EXPECT_EQ(executor.made_ready_for_delete.size(), 1);

}

namespace
{
	Task test_await_future_function(async::Future<int> fut, int* value, int* result)
	{
		*value = 1;
		int next_value = co_await std::move(fut);
		*result = next_value;
		*value = 2;
	}
}

TEST(testAsyncTask, testFuture)
{
	TestExecutor executor;
	int wait_value = 0;
	int result = 0;
	async::ThreadedPromise<int> promise;

	Task task = test_await_future_function(promise.get_future(), &wait_value, &result);

	EXPECT_EQ(wait_value, 0);
	executor.Dispatch(std::move(task));
	EXPECT_EQ(wait_value, 1);
	EXPECT_EQ(executor.made_ready.size(), 0);

	promise.set_value(5);
	EXPECT_EQ(executor.made_ready.size(), 1);
	executor.m_tasks[0].Resume();
	EXPECT_EQ(wait_value, 2);
	EXPECT_EQ(result, 5);
	EXPECT_TRUE(executor.m_tasks[0].Done());
	EXPECT_EQ(executor.made_ready_for_delete.size(), 1);

}

TEST(testAsyncTask, testFutureSetBeforeAwait)
{
	TestExecutor executor;
	int wait_value = 0;
	int result = 0;
	async::ThreadedPromise<int> promise;
	promise.set_value(5);

	Task task = test_await_future_function(promise.get_future(), &wait_value, &result);

	EXPECT_EQ(wait_value, 0);
	executor.Dispatch(std::move(task));

	EXPECT_EQ(executor.made_ready.size(), 0);
	EXPECT_EQ(wait_value, 2);
	EXPECT_EQ(result, 5);
	EXPECT_TRUE(executor.m_tasks[0].Done());
	EXPECT_EQ(executor.made_ready_for_delete.size(), 1);

}

namespace
{
	Task test_task_exception()
	{
		throw std::runtime_error("meeeh");
	}
	Task test_task_propagate_execption(int* value)
	{
		*value = 1;
		co_await test_task_exception();
	}
}

TEST(testAsyncTask, testExceptionPropagation)
{
	TestExecutor executor;
	int wait_value = 0;
	async::ThreadedPromise<int> promise;
	promise.set_value(5);

	Task task = test_task_propagate_execption(&wait_value);

	ASSERT_EQ(wait_value, 0);
	executor.Dispatch(std::move(task));

	EXPECT_EQ(wait_value, 1);
	ASSERT_EQ(executor.m_tasks.size(), 1);
	ASSERT_TRUE(executor.m_tasks[0].handle().promise().get_error());
	std::string exception_what;
	try
	{
		std::rethrow_exception(executor.m_tasks[0].handle().promise().get_error());
	}
	catch (const std::exception& e)
	{
		exception_what = e.what();
	}
	EXPECT_EQ(exception_what, "meeeh");
	EXPECT_EQ(executor.made_ready_for_delete.size(), 1);

}
