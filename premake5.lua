workspace "Nebula"
	architecture "x86_64"
	startproject "Nebula Storm"

	configurations {
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

--Include Directories (Relative to Solution Directory)
includedir = {}
includedir["spdlog"] = "%{wks.location}/Nebula/Modules/spdlog/include"
includedir["GLFW"] = "%{wks.location}/Nebula/Modules/glfw/include"
includedir["GLad"] = "%{wks.location}/Nebula/Modules/glad/include"
includedir["ImGui"] = "%{wks.location}/Nebula/Modules/imgui/include"
includedir["stb"] = "%{wks.location}/Nebula/Modules/stb_image"
includedir["entt"] = "%{wks.location}/Nebula/Modules/entt/include"
includedir["yaml"] = "%{wks.location}/Nebula/Modules/yaml-cpp/include"
includedir["imguizmo"] = "%{wks.location}/Nebula/Modules/imguizmo"

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