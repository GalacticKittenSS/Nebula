include "premake/solution_items.lua"
include "Dependencies.lua"

workspace "Nebula"
	architecture "x86_64"
	startproject "Nebula Storm"

	configurations {
		"Debug",
		"Release",
		"Dist"
	}

	flags {
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

--Dependencies
group "Dependencies"
	include "premake"
	include "Nebula/Modules/Box2D"
	include "Nebula/Modules/GLFW"
	include "Nebula/Modules/GLad"
	include "Nebula/Modules/msdf-atlas-gen"
	include "Nebula/Modules/yaml-cpp"

	--ImGui Submodule no longer contains premake5.lua
	--include "Nebula/Modules/ImGui"

	project "ImGui"
		kind "StaticLib"
		language "C++"
		staticruntime "off"

		targetdir ("bin/" .. outputdir .. "/%{prj.name}")
		objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

		files {
			"%{includedir.ImGui}/imconfig.h",
			"%{includedir.ImGui}/imgui.h",
			"%{includedir.ImGui}/imgui.cpp",
			"%{includedir.ImGui}/imgui_draw.cpp",
			"%{includedir.ImGui}/imgui_internal.h",
			"%{includedir.ImGui}/imgui_tables.cpp",
			"%{includedir.ImGui}/imgui_widgets.cpp",
			"%{includedir.ImGui}/imstb_rectpack.h",
			"%{includedir.ImGui}/imstb_textedit.h",
			"%{includedir.ImGui}/imstb_truetype.h",
			"%{includedir.ImGui}/imgui_demo.cpp"
		}

		filter "system:windows"
			systemversion "latest"
			cppdialect "C++17"

		filter "system:linux"
			pic "On"
			systemversion "latest"
			cppdialect "C++17"

		filter "configurations:Debug"
			runtime "Debug"
			symbols "on"

		filter "configurations:Release"
			runtime "Release"
			optimize "on"

		filter "configurations:Dist"
			runtime "Release"
			optimize "on"
			symbols "off"
group ""

--The Nebula Engine
group "Core"
	include "Nebula"
	include "Nebula-ScriptCore"
group ""

--The Nebula Editor
include "Nebula-Storm"