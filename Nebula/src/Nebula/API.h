#pragma once

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

#ifdef NB_ENABLE_ASSERTS
	#define NB_ASSERT(x, ...) { if(!(x)) { NB_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define CL_ASSERT(x, ...) { if(!(x)) { CL_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define NB_ASSERT(x, ...)
	#define CL_ASSERT(x, ...)
#endif //NB_ENABLE_ASSERTS

#define BIT(x) (1 << x)