#include "LogHandler.hpp"
#include "toolgui/Application.hpp"
#include "toolgui/LogView.hpp"
#include <chrono>

node::LogHandler::LogHandler(Application* app)
	:m_app{app}
{
}

node::LogHandler::~LogHandler()
{
}

void node::LogHandler::LogSDLMessage(void* userdata, int category, SDL_LogPriority priority, const char* message)
{
	LogHandler* logger = static_cast<LogHandler*>(userdata);
	assert(logger->m_default_logger);
	auto formatted_message = std::format("[{:%H:%M:%S}] {}", std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()), message);
	logger->m_default_logger(nullptr, category, priority, formatted_message.c_str());
	if (priority < SDL_LOG_PRIORITY_INFO)
	{
		return;
	}
	logger->AddMessage(std::move(formatted_message), category, priority);
}

void node::LogHandler::AddMessage(std::string message, int category, SDL_LogPriority priority)
{
	UNUSED_PARAM(category);
	UNUSED_PARAM(priority);

	std::lock_guard g{ m_mutex };
	m_logs.push_back({ std::move(message), category, priority });
	SetEvent();
}

void node::LogHandler::DispatchLogs()
{
	std::vector<logging::LogLine> result;
	{
		std::lock_guard g{ m_mutex };
		result.reserve(m_logs.size());
		for (auto&& item : m_logs)
		{
			result.push_back(std::move(item));
		}
		m_logs.clear();
		ClearEvent();
	}
	if (auto* logView = m_logView.GetObjectPtr())
	{
		for (auto& log_line : result)
		{
			logView->AddLine(std::move(log_line));
		}
	}
}

void node::LogHandler::SetDefaultLogger(SDL_LogOutputFunction default_logger)
{
	m_default_logger = default_logger;
}

void node::LogHandler::SetLogView(LogView* logView)
{
	if (logView != m_logView.GetObjectPtr())
	{
		m_logView = HandlePtrS<LogView, Widget>{};
	}
	if (logView)
	{
		m_logView = HandlePtrS<LogView, Widget>{ *logView };
	}
}

void node::LogHandler::SetEvent()
{
	if (!m_update_requested)
	{
		assert(m_app);
		m_app->AddMainThreadTask([] {});
		m_update_requested = true;
	}
}

void node::LogHandler::ClearEvent()
{
	m_update_requested = false;
}
