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
