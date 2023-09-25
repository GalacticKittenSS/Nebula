#pragma once

//------CORE----------
#include "Nebula/Core/API.h"
#include "Nebula/Core/UUID.h"
#include "Nebula/Core/Buffer.h"
#include "Nebula/Core/FileSystem.h"
//--------------------

//----LAYERS---------
#include "Nebula/Core/Layer.h"
#include "Nebula/imgui/ImGui_Layer.h"
//--------------------

//-----MATHS----------
#include "Nebula/Maths/Maths.h"
//--------------------

//----RENDERER--------
#include "Nebula/Renderer/Camera.h"
#include "Nebula/Renderer/Texture.h"
#include "Nebula/Renderer/Fonts.h"
#include "Nebula/Renderer/FrameBuffer.h"

//--Renderers
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
#include "Nebula/Utils/Time.h"
#include "Nebula/Utils/Arrays.h"
#include "Nebula/Utils/Random.h"
#include "Nebula/Utils/Platform_Utils.h"
//--------------------

//----SCRIPTS---------
#include "Nebula/Scripting/ScriptEngine.h"
//--------------------

//----PROJECT---------
#include "Nebula/Project/Project.h"
//--------------------

//----AssetManager----
#include "Nebula/AssetManager/AssetManager.h"
//--------------------

//----SCENES----------
#include "Nebula/Scene/Scene.h"
#include "Nebula/Scene/Entity.h"
#include "Nebula/Scene/Components.h"
#include "Nebula/Scene/Scene_Serializer.h"
#include "Nebula/Scene/SceneRenderer.h"
//--------------------

//----Application-----
#include "Nebula/Core/Window.h"
#include "Nebula/Core/Application.h"
//--------------------