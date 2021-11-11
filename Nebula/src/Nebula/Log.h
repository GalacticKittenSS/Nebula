#pragma once

#include "API.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/fmt/ostr.h>

namespace Nebula {
	class NB_API Log {
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

#ifdef NEBULA
	//Nebula Log Macros
	#define NB_CRITICAL(...) ::Nebula::Log::GetCoreLogger()->critical(__VA_ARGS__)
	#define NB_ERROR(...)	 ::Nebula::Log::GetCoreLogger()->error(__VA_ARGS__)
	#define NB_WARN(...)	 ::Nebula::Log::GetCoreLogger()->warn(__VA_ARGS__)
	#define NB_INFO(...)	 ::Nebula::Log::GetCoreLogger()->info(__VA_ARGS__)
	#define NB_TRACE(...)	 ::Nebula::Log::GetCoreLogger()->trace(__VA_ARGS__)
	//--------------
#else
	//Client Log Macros
	#define NB_CRITICAL(...) ::Nebula::Log::GetClientLogger()->critical(__VA_ARGS__)
	#define NB_ERROR(...)	 ::Nebula::Log::GetClientLogger()->error(__VA_ARGS__)
	#define NB_WARN(...)	 ::Nebula::Log::GetClientLogger()->warn(__VA_ARGS__)
	#define NB_INFO(...)	 ::Nebula::Log::GetClientLogger()->info(__VA_ARGS__)
	#define NB_TRACE(...)	 ::Nebula::Log::GetClientLogger()->trace(__VA_ARGS__)
	//--------------
#endif