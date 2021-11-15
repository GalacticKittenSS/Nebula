#include "nbpch.h"
#include "Render_Command.h"

#include "Platform/OpenGl/OpenGL_RendererAPI.h"

namespace Nebula {
	Scope<RendererAPI> RenderCommand::s_RendererAPI = RendererAPI::Create();
}