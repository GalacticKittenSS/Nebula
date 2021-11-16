#include "nbpch.h"
#include "Input.h"

#ifdef NB_WINDOWS
	#include "Platform/Windows/Win_Input.h"
#endif

namespace Nebula {

	Scope<Input> Input::s_Instance = Input::Create();

	Scope<Input> Input::Create()
	{
#ifdef NB_WINDOWS
		return CreateScope<Win_Input>();
#else
		NB_ASSERT(false, "Unknown platform!");
		return nullptr;
#endif
	}
}