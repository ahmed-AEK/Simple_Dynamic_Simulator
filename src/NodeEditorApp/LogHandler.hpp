#pragma once

#include <mutex>
#include <vector>
#include <string>

namespace node
{
	namespace logging
	{
		struct LogLine;
	}
	class Application;
	class LogView;
	class LogHandler
	{
	public:
		LogHandler(Application* app);
		~LogHandler();
		LogHandler(const LogHandler&) = delete;
		LogHandler& operator=(const LogHandler&) = delete;

		static void LogSDLMessage(void* userdata, int category, SDL_LogPriority priority, const char* message);

		void AddMessage(std::string message, int category, SDL_LogPriority priority);

		void DispatchLogs();
		void SetDefaultLogger(SDL_LogOutputFunction default_logger);
		void SetLogView(LogView* logView);
	private:

		void SetEvent();
		void ClearEvent();
		std::vector<logging::LogLine> m_logs;
		std::mutex m_mutex;
		Application* m_app = nullptr;
		SDL_LogOutputFunction m_default_logger = nullptr;
		HandlePtrS<LogView, Widget> m_logView;

		bool m_update_requested = false;
	};
}