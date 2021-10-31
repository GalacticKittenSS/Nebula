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

	pchheader "nbpch.h"
	pchsource "nbpch.cpp"

	files {
		"%{prj.name}/src/Nebula/**.h",
		"%{prj.name}/src/Nebula/**.cpp",
		"%{prj.name}/src/nbpch.h",
		"%{prj.name}/src/nbpch.cpp",
		"%{prj.name}/src/Nebula.h",
		"%{prj.name}/include/Nebula.h"
	}

	includedirs {
		"%{prj.name}/src",
		"%{prj.name}/src/Modules/spdlog/include"
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
		"Nebula/include"
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