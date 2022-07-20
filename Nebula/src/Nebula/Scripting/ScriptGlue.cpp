#include "nbpch.h"
#include "ScriptGlue.h"

#include <mono/metadata/object.h>

namespace Nebula {

#define NB_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Nebula.InternalCalls::" #Name, Name);

	static void NativeLog(MonoString* text, int parameter)
	{
		char* cStr = mono_string_to_utf8(text);
		std::string str(cStr);
		mono_free(cStr);

		NB_TRACE("{}, {}", cStr, parameter);
	}

	static void NativeLog_Vector(vec3* parameter, vec3* result)
	{
		NB_TRACE("Vector 3 ({}, {}, {})", parameter->x, parameter->y, parameter->z);
		*result = normalize(*parameter);
	}

	static float NativeLog_VectorDot(vec3* parameter)
	{
		NB_TRACE("Vector 3 ({}, {}, {})", parameter->x, parameter->y, parameter->z);
		return dot(*parameter, *parameter);
	}

	void ScriptGlue::RegisterFunctions() {
		NB_ADD_INTERNAL_CALL(NativeLog);
		NB_ADD_INTERNAL_CALL(NativeLog_Vector);
		NB_ADD_INTERNAL_CALL(NativeLog_VectorDot);
	}
}