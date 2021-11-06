workspace "Nebula"
	architecture "x64"
	startproject "Tests"

	configurations {
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

--Include Directories (Relative to Nebula Project)
includedir = {}
includedir["GLFW"] = "Nebula/Modules/glfw/include"
includedir["GLad"] = "Nebula/Modules/glad/include"
includedir["ImGui"] = "Nebula/Modules/imgui"

group "Dependencies"
	include "Nebula/Modules/GLFW"
	include "Nebula/Modules/GLad"
	include "Nebula/Modules/ImGui"
group ""

project "Nebula"
	location "Nebula"
	kind "SharedLib"
	language "C++"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-obj/" .. outputdir .. "/%{prj.name}")

	pchheader "nbpch.h"
	pchsource "nbpch.cpp"

	files {
		"%{prj.name}/src/Nebula/**.h",
		"%{prj.name}/src/Nebula/**.cpp",
		"%{prj.name}/src/nbpch.h",
		"%{prj.name}/src/nbpch.cpp",
		"%{prj.name}/src/Nebula.h",
		"%{prj.name}/src/Platform/**.h",
		"%{prj.name}/src/Platform/**.cpp",
		"%{prj.name}/include/**.h"
	}

	includedirs {
		"%{prj.name}/src",
		"%{prj.name}/Modules/spdlog/include",
		"%{includedir.GLFW}",
		"%{includedir.GLad}",
		"%{includedir.ImGui}"
	}

	links { 
		"GLFW",
		"GLad",
		"ImGui",
		"opengl32.lib"
	}

	defines "NEBULA"

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"

		defines "NB_WINDOWS"

		postbuildcommands {
			("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/Tests/\"")
		}

	filter "configurations:Debug"
		defines "NB_DEBUG"
		runtime "Debug"
		buildoptions '/Yc"nbpch.h"'
		symbols "On"

		defines "NB_ENABLE_ASSERTS"

	filter "configurations:Release"
		defines "NB_DEBUG"
		runtime "Release"
		buildoptions '/Yc"nbpch.h"'
		optimize "On"

	filter "configurations:Dist"
		defines "NB_DIST"
		runtime "Release"
		buildoptions '/Yc"nbpch.h"'
		optimize "On"

project "Tests"
	location "Tests"
	kind "ConsoleApp"
	language "C++"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-obj/" .. outputdir .. "/%{prj.name}")

	files {
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs {
		"Nebula/include",
		"Nebula/Modules/spdlog/include",
		"%{includedir.GLFW}",
		"%{includedir.GLad}",
		"%{includedir.ImGui}"
	}

	links {
		"Nebula"
	}

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"

		defines {
			"NB_WINDOWS"
		}

	filter "configurations:Debug"
		defines "NB_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "NB_DEBUG"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "NB_DIST"
		runtime "Release"
		optimize "On"