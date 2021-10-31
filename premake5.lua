workspace "Nebula"
	architecture "x64"

	configurations {
		"Debug",
		"Release",
		"Dist"
	}

outdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Nebula"
	location "Nebula"
	kind "SharedLib"
	language "C++"

	targetdir ("bin/" .. outdir .. "/%{prj.name}")
	objdir ("bin-obj/" .. outdir .. "/%{prj.name}")

	files {
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/include/Nebula.h"
	}

	includedirs {
		"Nebula\src",
		"Nebula\src\Modules\spdlog"
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
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outdir .. "/Tests")
		}

	filter "configurations:Debug"
		defines "NB_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "NB_DEBUG"
		optimize "On"

	filter "configurations:Dist"
		defines "NB_DIST"
		optimize "On"

project "Tests"
	location "Tests"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outdir .. "/%{prj.name}")
	objdir ("bin-obj/" .. outdir .. "/%{prj.name}")

	files {
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs {
		"Nebula\include"
		"Nebula\src\Modules\spdlog"
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
		symbols "On"

	filter "configurations:Release"
		defines "NB_DEBUG"
		optimize "On"

	filter "configurations:Dist"
		defines "NB_DIST"
		optimize "On"