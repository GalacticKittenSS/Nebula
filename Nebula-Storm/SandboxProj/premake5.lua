local NebulaRootDir = "../.."

include (NebulaRootDir .. "/premake/solution_items.lua")

workspace "Sandbox"
	architecture "x86_64"
	startproject "Sandbox"

	configurations {
		"Debug",
		"Release",
		"Dist"
	}

	flags {
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Sandbox"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"
	staticruntime "off"

	targetdir ("Binaries")
	objdir ("Intermediates")

	files {
		"Assets/**.cs",
	}

	links {
		"Nebula-ScriptCore"
	}

	filter "configurations:Debug"
		optimize "Off"
		symbols "Default"
		
	filter "configurations:Release"
		optimize "On"
		symbols "Default"

	filter "configurations:Dist"
		optimize "Full"
		symbols "Off"

group "Nebula"
	include (NebulaRootDir .. "/Nebula-ScriptCore")
group ""