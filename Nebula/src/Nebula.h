#pragma once

//----API DEFINES-----
#include "Nebula/Core/API.h"
//--------------------

//----LAYERS---------
#include "Nebula/Core/Layer.h"
#include "Nebula/imgui/ImGui_Layer.h"
//--------------------

//----SCENES----------
#include "Nebula/Scene/Scene.h"
#include "Nebula/Scene/Entity.h"
#include "Nebula/Scene/Components.h"
#include "Nebula/Scene/Scene_Serializer.h"
//--------------------

//-----MATHS----------
#include "Nebula/Maths/Maths.h"
//--------------------

//----RENDERER--------
#include "Nebula/Renderer/Shader.h"
#include "Nebula/Renderer/Texture.h"
#include "Nebula/Renderer/GameObjects.h"
#include "Nebula/renderer/Camera_Controller.h"
#include "Nebula/Renderer/FrameBuffer.h"

//--Buffers
#include "Nebula/Renderer/Buffer.h"
#include "Nebula/Renderer/Vertex_Array.h"

//--Renderers
#include "Nebula/Renderer/Renderer.h"
#include "Nebula/Renderer/Renderer2D.h"
#include "Nebula/Renderer/Render_Command.h"
//--------------------

//----EVENTS---------
#include "Nebula/Core/Input.h"
#include "Nebula/Events/Event.h"
#include "Nebula/Events/Key_Event.h"
#include "Nebula/Events/Mouse_Event.h"
#include "Nebula/Events/Window_Event.h"
//--------------------

//----UTILS-----------
#include "Nebula/Utils/Platform_Utils.h"
#include "Nebula/Utils/Arrays.h"
#include "Nebula/Utils/Random.h"
#include "Nebula/Utils/Time.h"
//--------------------

//----Application-----
#include "Nebula/Core/Window.h"
#include "Nebula/Core/Application.h"
//--------------------