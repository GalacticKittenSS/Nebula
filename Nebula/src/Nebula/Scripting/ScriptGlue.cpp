#include "nbpch.h"
#include "ScriptGlue.h"

#include "ScriptEngine.h"

#include "Nebula/Scene/Scene.h"
#include "Nebula/Core/Input.h"

#include <mono/metadata/object.h>

namespace Nebula {

#define NB_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Nebula.InternalCalls::" #Name, Name);

	static std::string GetStringFromMono(MonoString* text)
	{
		char* cStr = mono_string_to_utf8(text);
		std::string str(cStr);
		mono_free(cStr);

		return str;
	}

	static void Native_Log(int level, MonoString* text)
	{
		std::string str = GetStringFromMono(text);
		
		switch (level)
		{
			case 0: NB_TRACE("[C# Script] {}", str); break;
			case 1: NB_WARN ("[C# Script] {}", str); break;
			case 2: NB_ERROR("[C# Script] {}", str); break;
		}
	}
	
	static void Entity_GetTranslation(UUID entityID, vec3* out)
	{
		Entity entity = { entityID, ScriptEngine::GetSceneContext() };
		*out = entity.GetTransform().Translation;
	}

	static void Entity_SetTranslation(UUID entityID, vec3* translation)
	{
		Entity entity = { entityID, ScriptEngine::GetSceneContext() };
		entity.GetTransform().Translation = *translation;
		CalculateGlobalTransform(entity);
	}

	static bool Input_IsKeyDown(KeyCode keycode)
	{
		return Input::IsKeyPressed(keycode);
	}

	void ScriptGlue::RegisterFunctions() {
		NB_ADD_INTERNAL_CALL(Native_Log);

		NB_ADD_INTERNAL_CALL(Entity_GetTranslation);
		NB_ADD_INTERNAL_CALL(Entity_SetTranslation);

		NB_ADD_INTERNAL_CALL(Input_IsKeyDown);
	}
}