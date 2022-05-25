#include "nbpch.h"
#include "Nebula/Core/Window.h"

#ifdef NB_WINDOWS
#include "Platform/Windows/Win_Window.h"
#endif

namespace Nebula {

	Scope<Window> Window::Create(const WindowProps& props)
	{
#ifdef NB_WINDOWS
		return CreateScope<Win_Window>(props);
#else
		NB_ASSERT(false, "Unknown platform!");
		return nullptr;
#endif
	}

}
