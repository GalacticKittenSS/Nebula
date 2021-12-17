#include "nbpch.h"
#include "Nebula/Utils/Platform_Utils.h"

#include "Nebula/Core/Application.h"

#include <commdlg.h>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace Nebula {
	std::string FileDialogs::OpenFile(const char* filter) {
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 }; 
		CHAR currentDir[256] = { 0 };

		ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		if (GetCurrentDirectoryA(256, currentDir))
			ofn.lpstrInitialDir = currentDir;
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		
		if (GetOpenFileNameA(&ofn) == TRUE) {
			return ofn.lpstrFile;
		}

		return std::string();
	}

	std::string FileDialogs::SaveFile(const char* filter) {
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		CHAR currentDir[256] = { 0 };

		ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		if (GetCurrentDirectoryA(256, currentDir))
			ofn.lpstrInitialDir = currentDir;
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.lpstrDefExt = std::strchr(filter, '\0') + 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

		if (GetSaveFileNameA(&ofn) == TRUE) {
			return ofn.lpstrFile;
		}

		return std::string();
	}
}