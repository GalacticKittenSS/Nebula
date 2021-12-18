--include "./vendor/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"

workspace "Nebula"
	architecture "x86_64"
	startproject "Nebula Storm"

	configurations {
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

--Dependencies
group "Dependencies"
	include "Nebula/Modules/GLFW"
	include "Nebula/Modules/GLad"
	include "Nebula/Modules/ImGui"
	include "Nebula/Modules/yaml-cpp"
group ""

--The Nebula Engine
include "Nebula"

--The Nebula Editor
include "Nebula-Storm"