project "Nebula Storm"
	kind "ConsoleApp"
	
	cppdialect "C++17"
	language "C++"

	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-obj/" .. outputdir .. "/%{prj.name}")

	files {
		"src/**.h",
		"src/**.cpp"
	}

	includedirs {
		"%{includedir.Nebula}",
		"%{includedir.Entt}",
		"%{includedir.glm}",
		"%{includedir.ImGui}",
		"%{includedir.ImGuizmo}",
		"%{includedir.Spdlog}"
	}

	links {
		"Nebula"
	}

	linkoptions { "-IGNORE:4099" }

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines  {
			"NB_DEBUG",
			"NB_ENABLE_ASSERTS"
		}
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