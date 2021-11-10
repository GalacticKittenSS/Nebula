#include "nbpch.h"
#include "Render_Command.h"

#include "Platform/OpenGl/OpenGL_RendererAPI.h"

namespace Nebula {
	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;
}