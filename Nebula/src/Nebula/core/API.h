#pragma once

#include <memory>

// Platform detection using predefined macros
#ifdef _WIN32
	/* Windows x64/x86 */
	#ifdef _WIN64
		/* Windows x64  */
		#define NB_WINDOWS
	#else
		/* Windows x86 */
		#error "x86 Builds are not supported!"
#endif
#elif defined(__ANDROID__)
	#define NB_ANDROID
	#error "Android is not supported!"
#else
	/* Unknown compiler/platform */
	#error "Unknown platform!"
#endif // End of platform detection


// DLL support
#ifdef NB_WINDOWS
	#ifdef NB_DLL
		#ifdef NEBULA
			#define NB_API		_declspec(dllexport)
			#define IMGUI_API	_declspec(dllexport)
		#else
			#define NB_API		_declspec(dllimport)
			#define IMGUI_API	_declspec(dllimport)
	#endif //NEBULA
	#else 
		#define NB_API 
	#endif //NB_DLL
	#else
		#error Only Windows is Supported
#endif //NB_WINDOWS

#ifdef NB_DEBUG
	#if defined(NB_WINDOWS)
		#define NB_DEBUGBREAK() __debugbreak()
	#else
		#error "Platform doesn't support debugbreak yet!"
	#endif
#endif //NB_DEBUG

#ifdef NB_ENABLE_ASSERTS
	#define NB_ASSERT(x, ...) { if(!(x)) { NB_ERROR("Assertion Failed: {0}", __VA_ARGS__); NB_DEBUGBREAK(); } }
	#define CL_ASSERT(x, ...) { if(!(x)) { CL_ERROR("Assertion Failed: {0}", __VA_ARGS__); NB_DEBUGBREAK(); } }
#else
	#define NB_ASSERT(x, ...)
	#define CL_ASSERT(x, ...)
#endif //NB_ENABLE_ASSERTS

#define BIT(x) (1 << x)

namespace Nebula {
	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ...args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
}