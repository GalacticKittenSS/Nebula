#pragma once

//----API DEFINES-----
#include "Nebula/API.h"
#include "Nebula/Key_Codes.h"
#include "Nebula/Mouse_Codes.h"
//--------------------

//----LAYERS---------
#include "Nebula/Layer.h"
#include "Nebula/imgui/ImGui_Layer.h"
//--------------------

//----BUFFERS---------
#include "Nebula/renderer/Shader.h"
#include "Nebula/renderer/Buffer.h"
#include "Nebula/renderer/Vertex_Array.h"
//--------------------

#include "Nebula/renderer/Texture.h"

//----RENDERER--------
#include "Nebula/renderer/Renderer.h"
#include "Nebula/renderer/Render_Command.h"
//--------------------

//----EVENTS---------
#include "Nebula/Input.h"
#include "Nebula/events/Event.h"
#include "Nebula/events/Key_Event.h"
#include "Nebula/events/Mouse_Event.h"
#include "Nebula/events/Window_Event.h"
//--------------------

//-----CAMERA----------
#include "Nebula/renderer/Camera.h"
//--------------------

#include "Nebula/core/Timestep.h"
#include "Nebula/Window.h"
#include "Platform/OpenGl/OpenGL_Shader.h"

//----Application-----
#include "Nebula/Application.h"
//--------------------

/*	-----Entry Point-------
	HAS TO BE INCLUDED BELOW
		APPLICATION
	-----------------------	*/
#include "Nebula/Entry_Point.h"