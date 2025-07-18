#pragma once

#include <format>
#include "SDL_Framework/SDL_headers.h"

namespace node
{

template<typename T>
auto unmove(T&& v) -> std::remove_reference_t<T>& {
	return static_cast<std::remove_reference_t<T>&>(v);
}

namespace logging
{
enum class LogCategory
{
	General = SDL_LOG_CATEGORY_CUSTOM,
	Internal,
	Core,
	GUI,
	Simulator,
	Extension,
};

class Logger
{
public:
	Logger(LogCategory category) : m_category{ static_cast<uint8_t>(category) } {
		assert(static_cast<int>(category) >= 0);
		assert(static_cast<int>(category) < 256);
	}

	template <class... Args>
	void LogInfo(std::format_string<Args...> fmt, Args&&... args) const
	{
		Log(SDL_LOG_PRIORITY_INFO, std::move(fmt), std::forward<Args>(args)...);
	}

	template <class... Args>
	void LogError(std::format_string<Args...> fmt, Args&&... args) const
	{
		Log(SDL_LOG_PRIORITY_ERROR, std::move(fmt), std::forward<Args>(args)...);
	}

	template <class... Args>
	void LogDebug(std::format_string<Args...> fmt, Args&&... args) const
	{
		Log(SDL_LOG_PRIORITY_DEBUG, std::move(fmt), std::forward<Args>(args)...);
	}

	template <class... Args>
	void LogTrace(std::format_string<Args...> fmt, Args&&... args) const
	{
		Log(SDL_LOG_PRIORITY_TRACE, std::move(fmt), std::forward<Args>(args)...);
	}

	template <class... Args>
	void LogCritical(std::format_string<Args...> fmt, Args&&... args) const
	{
		Log(SDL_LOG_PRIORITY_CRITICAL, std::move(fmt), std::forward<Args>(args)...);
	}

	template <class... Args>
	void LogWarn(std::format_string<Args...> fmt, Args&&... args) const
	{
		Log(SDL_LOG_PRIORITY_WARN, std::move(fmt), std::forward<Args>(args)...);
	}

	template <class... Args>
	void LogVerbose(std::format_string<Args...> fmt, Args&&... args) const
	{
		Log(SDL_LOG_PRIORITY_VERBOSE, std::move(fmt), std::forward<Args>(args)...);
	}

	template <class... Args>
	void Log(SDL_LogPriority priority, std::format_string<Args...> fmt, Args&&... args) const
	{
		auto lowest_priority = SDL_GetLogPriority(static_cast<int>(m_category));
		if (priority < lowest_priority)
		{
			return; // short-circuit turning into string
		}
		SDL_LogOutputFunction callback{};
		void *userdata{};
		SDL_GetLogOutputFunction(&callback, &userdata);
		if constexpr (sizeof...(Args) == 0)
		{
			callback(userdata, static_cast<int>(m_category), priority, fmt.get().data());
		}
		else
		{

			callback(userdata, static_cast<int>(m_category), priority, std::format(fmt, std::forward<Args>(args)...).c_str());
		}
	}

private:

	uint8_t m_category = static_cast<uint8_t>(LogCategory::General);
};
	
}

static inline logging::Logger logger(logging::LogCategory category)
{
	return logging::Logger{ category };
}
}