project "Nebula"
	kind "StaticLib"
	cppdialect "C++17"
	language "C++"
	staticruntime "on"

	pchheader "nbpch.h"
	pchsource "src/nbpch.cpp"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-obj/" .. outputdir .. "/%{prj.name}")

	files {
		"src/**.h",
		"src/**.cpp",
		"include/**.h",

		"Modules/stb_image/*.h",
		"Modules/stb_image/*.cpp",

		"Modules/imgui/src/*.cpp",

		"Modules/imguizmo/ImGuizmo.h",
		"Modules/imguizmo/ImGuizmo.cpp"
	}

	defines {
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",
		"NEBULA"
	}

	includedirs {
		"src",
		"%{includedir.spdlog}",
		"%{includedir.GLFW}",
		"%{includedir.GLad}",
		"%{includedir.ImGui}",
		"%{includedir.stb}",
		"%{includedir.entt}",
		"%{includedir.yaml}",
		"%{includedir.imguizmo}"
	}

	links { 
		"GLFW",
		"GLad",
		"ImGui",
		"yaml-cpp",
		"opengl32.lib"
	}
	
	filter "files:Modules/imguizmo/**.cpp"
	flags { "NoPCH" }

	filter "files:Modules/imgui/src/**.cpp"
	flags { "NoPCH" }

	filter "system:windows"
		systemversion "latest"

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