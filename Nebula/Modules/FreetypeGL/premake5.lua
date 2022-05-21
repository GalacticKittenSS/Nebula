project "FreetypeGL"
	kind "StaticLib"
	language "C++"
	cppdialect "C++11"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files {
		"include/**.h",
		"src/**.c",
		"Freetype/include/**.h"
	}

	includedirs {
		"include",
		"Freetype/include",
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

		links {
			"%{Library.Freetype_Debug}"
		}

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

		links {
			"%{Library.Freetype}"
		}

	filter "configurations:Dist"
		runtime "Release"
		optimize "on"

		links {
			"%{Library.Freetype}"
		}