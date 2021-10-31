#pragma once

#ifdef NB_WINDOWS
	#ifdef NEBULA
		#define NB_API _declspec(dllexport)
	#else
		#define NB_API _declspec(dllimport)
	#endif //NEBULA
#else
	#error Only Windows is Supported
#endif //NB_WINDOWS