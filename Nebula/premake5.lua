project "Nebula"
	kind "StaticLib"
	cppdialect "C++17"
	language "C++"
	staticruntime "off"

	pchheader "nbpch.h"
	pchsource "src/nbpch.cpp"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-obj/" .. outputdir .. "/%{prj.name}")

	files {
		"src/**.h",
		"src/**.cpp",
		
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
		"%{includedir.Box2D}",
		"%{includedir.Entt}",
		"%{includedir.Freetype}",
		"%{includedir.FileWatch}",
		"%{includedir.GLFW}",
		"%{includedir.GLad}",
		"%{includedir.glm}",
		"%{includedir.ImGui}",
		"%{includedir.ImGuizmo}",
		"%{includedir.mono}",
		"%{includedir.Spdlog}",
		"%{includedir.Stb}",
		"%{includedir.Yaml}",
		"%{includedir.VulkanSDK}"
	}

	links { 
		"Box2D",
		"Freetype",
		"GLad",
		"GLFW",
		"ImGui",
		"yaml-cpp",
		"opengl32.lib",

		"%{Library.mono}"
	}
	
	filter "files:Modules/imguizmo/**.cpp"
	flags { "NoPCH" }

	filter "files:Modules/imgui/src/**.cpp"
	flags { "NoPCH" }

	filter "files:Modules/Freetype/**.c"
	flags { "NoPCH" }

	filter "system:windows"
		systemversion "latest"

		links {
			"%{Library.WinSock}",
			"%{Library.WinMM}",
			"%{Library.WinVersion}",
			"%{Library.BCrypt}"
		}

	filter "configurations:Debug"
		defines "NB_DEBUG"
		runtime "Debug"
		symbols "on"
		
		links {
			"%{Library.ShaderC_Debug}",
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}"
		}

		defines "NB_ENABLE_ASSERTS"

	filter "configurations:Release"
		defines "NB_RELEASE"
		runtime "Release"
		optimize "on"

		links {
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}

	filter "configurations:Dist"
		defines "NB_DIST"
		runtime "Release"
		optimize "on"
		
		links {
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}