workspace "Nebula"
	architecture "x64"

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

include "Nebula/Modules/GLFW"
include "Nebula/Modules/GLad"

project "Nebula"
	location "Nebula"
	kind "SharedLib"
	language "C++"

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
		"%{includedir.GLad}"
	}

	links { 
		"GLFW",
		"GLad",
		"opengl32.lib"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines {
			"NB_WINDOWS",
			"NEBULA"
		}

		postbuildcommands {
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Tests")
		}

	filter "configurations:Debug"
		defines "NB_DEBUG"
		buildoptions '/MDd /Yc"nbpch.h"'
		symbols "On"

	filter "configurations:Release"
		defines "NB_DEBUG"
		buildoptions '/MD /Yc"nbpch.h"'
		optimize "On"

	filter "configurations:Dist"
		defines "NB_DIST"
		buildoptions '/MD /Yc"nbpch.h"'
		optimize "On"

project "Tests"
	location "Tests"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-obj/" .. outputdir .. "/%{prj.name}")

	files {
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs {
		"Nebula/include",
		"Nebula/Modules/spdlog/include",
		"%{includedir.GLFW}"
	}

	links {
		"Nebula"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines {
			"NB_WINDOWS"
		}

	filter "configurations:Debug"
		defines "NB_DEBUG"
		buildoptions "/MDd"
		symbols "On"

	filter "configurations:Release"
		defines "NB_DEBUG"
		buildoptions "/MD"
		optimize "On"

	filter "configurations:Dist"
		defines "NB_DIST"
		buildoptions "/MD"
		optimize "On"