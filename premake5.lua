workspace "Nebula"
	architecture "x64"
	startproject "Tests"

	configurations {
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

--Include Directories (Relative to Solution Directory)
includedir = {}
includedir["spdlog"] = "Nebula/Modules/spdlog/include"
includedir["GLFW"] = "Nebula/Modules/glfw/include"
includedir["GLad"] = "Nebula/Modules/glad/include"
includedir["ImGui"] = "Nebula/Modules/imgui/include"
includedir["glm"] = "Nebula/Modules/glm/glm"

--Dependencies
group "Dependencies"
	include "Nebula/Modules/GLFW"
	include "Nebula/Modules/GLad"
	include "Nebula/Modules/ImGui"
group ""

--The Nebula Engine
project "Nebula"
	location "Nebula"
	kind "StaticLib"

	cppdialect "C++17"
	language "C++"
	
	staticruntime "on"

	buildoptions '/Yc"nbpch.h"'

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-obj/" .. outputdir .. "/%{prj.name}")

	pchheader "nbpch.h"
	pchsource "nbpch.cpp"

	files {
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/include/**.h"
	}

	includedirs {
		"%{prj.name}/src",
		"%{includedir.spdlog}",
		"%{includedir.GLFW}",
		"%{includedir.GLad}",
		"%{includedir.ImGui}",
		"%{includedir.glm}"
	}

	links { 
		"GLFW",
		"GLad",
		"ImGui",
		"opengl32.lib"
	}

	defines {
		"_CRT_SECURE_NO_WARNINGS",
		"NEBULA"
	}

	filter "system:windows"
		systemversion "latest"

		defines "NB_WINDOWS"

	filter "configurations:Debug"
		defines "NB_DEBUG"
		runtime "Debug"
		symbols "on"

		defines "NB_ENABLE_ASSERTS"

	filter "configurations:Release"
		defines "NB_DEBUG"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "NB_DIST"
		runtime "Release"
		optimize "on"

--The Nebula Client
project "Tests"
	location "Tests"
	kind "ConsoleApp"
	
	cppdialect "C++17"
	language "C++"

	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-obj/" .. outputdir .. "/%{prj.name}")

	files {
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs {
		"Nebula/include",
		"%{includedir.spdlog}",
		"%{includedir.ImGui}"
	}

	links {
		"Nebula"
	}

	filter "system:windows"
		systemversion "latest"

		defines "NB_WINDOWS"

	filter "configurations:Debug"
		defines "NB_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "NB_DEBUG"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "NB_DIST"
		runtime "Release"
		optimize "on"
