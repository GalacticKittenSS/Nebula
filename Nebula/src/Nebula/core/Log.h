#pragma once

#include "API.h"

#include <spdlog/spdlog.h>

namespace Nebula {
	class NB_API Log {
	public:
		static void Init();

		inline static Ref<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static Ref<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static Ref<spdlog::logger> s_CoreLogger;
		static Ref<spdlog::logger> s_ClientLogger;
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