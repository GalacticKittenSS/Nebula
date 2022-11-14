include "premake/solution_items.lua"
include "Dependencies.lua"

workspace "Nebula"
	architecture "x86_64"
	startproject "Nebula Storm"

	configurations {
		"Debug",
		"Release",
		"Dist"
	}

	flags {
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

--Dependencies
group "Dependencies"
	include "premake"
	include "Nebula/Modules/Box2D"
	include "Nebula/Modules/GLFW"
	include "Nebula/Modules/GLad"
	include "Nebula/Modules/ImGui"
	include "Nebula/Modules/yaml-cpp"
	include "Nebula/Modules/FreetypeGL"
group ""

--The Nebula Engine
group "Core"
	include "Nebula"
	include "Nebula-ScriptCore"
group ""

--The Nebula Editor
include "Nebula-Storm"