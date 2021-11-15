#pragma once

//----API DEFINES-----
#include "Nebula/Core/API.h"
#include "Nebula/Core/Key_Codes.h"
#include "Nebula/Core/Mouse_Codes.h"
//--------------------

//----LAYERS---------
#include "Nebula/Core/Layer.h"
#include "Nebula/imgui/ImGui_Layer.h"
//--------------------

//----BUFFERS---------
#include "Nebula/Renderer/Shader.h"
#include "Nebula/Renderer/Buffer.h"
#include "Nebula/Renderer/Vertex_Array.h"
//--------------------

#include "Nebula/Renderer/Texture.h"

//----RENDERER--------
#include "Nebula/Renderer/Renderer.h"
#include "Nebula/Renderer/Render_Command.h"
//--------------------

//----EVENTS---------
#include "Nebula/Core/Input.h"
#include "Nebula/Events/Event.h"
#include "Nebula/Events/Key_Event.h"
#include "Nebula/Events/Mouse_Event.h"
#include "Nebula/Events/Window_Event.h"
//--------------------

//-----CAMERA----------
#include "Nebula/renderer/Camera_Controller.h"
//--------------------

#include "Nebula/Core/Timestep.h"
#include "Nebula/Core/Window.h"
#include "Platform/OpenGl/OpenGL_Shader.h"

//----Application-----
#include "Nebula/Core/Application.h"
//--------------------

/*	-----Entry Point-------
	HAS TO BE INCLUDED BELOW
		APPLICATION
	-----------------------	*/
#include "Nebula/Core/Entry_Point.h"