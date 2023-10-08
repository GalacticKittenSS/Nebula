-- Nebula Dependencies

VULKAN_SDK = os.getenv("VULKAN_SDK")

includedir = {}
includedir["Nebula"] =		"%{wks.location}/Nebula/src"
includedir["Box2D"] =		"%{wks.location}/Nebula/Modules/Box2D/include"
includedir["Entt"] =		"%{wks.location}/Nebula/Modules/entt/include"
includedir["FileWatch"] =	"%{wks.location}/Nebula/Modules/filewatch"
includedir["GLad"] =		"%{wks.location}/Nebula/Modules/glad/include"
includedir["GLFW"] =		"%{wks.location}/Nebula/Modules/glfw/include"
includedir["glm"] =			"%{wks.location}/Nebula/Modules/glm"
includedir["ImGui"] =		"%{wks.location}/Nebula/Modules/imgui"
includedir["ImGuizmo"] =	"%{wks.location}/Nebula/Modules/imguizmo"
includedir["Spdlog"] =		"%{wks.location}/Nebula/Modules/spdlog/include"
includedir["Stb"] =			"%{wks.location}/Nebula/Modules/stb_image"
includedir["Yaml"] =		"%{wks.location}/Nebula/Modules/yaml-cpp/include"
includedir["mono"] =	    "%{wks.location}/Nebula/Modules/Mono/include"
includedir["msdfgen"] =		"%{wks.location}/Nebula/Modules/msdf-atlas-gen/msdfgen"
includedir["msdf_atlas_gen"] = "%{wks.location}/Nebula/Modules/msdf-atlas-gen/msdf-atlas-gen"

includedir["shaderc"] =		"%{wks.location}/Nebula/Modules/shaderc/include"
includedir["SPIRV_Cross"] = "%{wks.location}/Nebula/Modules/SPIRV-Cross"
includedir["VulkanSDK"] = "	 %{VULKAN_SDK}/Include"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["mono_Debug"] = "%{wks.location}/Nebula/Modules/Mono/lib/Debug"
LibraryDir["mono_Release"] = "%{wks.location}/Nebula/Modules/Mono/lib/Release"

Library = {}
Library["mono_Debug"] = "%{LibraryDir.mono_Debug}/libmono-static-sgen.lib"
Library["mono_Release"] = "%{LibraryDir.mono_Release}/libmono-static-sgen.lib"

Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK}/SPIRV-Toolsd.lib"

Library["Freetype_Debug"] = "%{wks.location}/Nebula/Modules/FreetypeGL/Freetype/Freetype_Debug.lib"
Library["Freetype"] = "%{wks.location}/Nebula/Modules/FreetypeGL/Freetype/Freetype.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"

Library["Freetype_Debug"] = "%{wks.location}/Nebula/Modules/FreetypeGL/Freetype/Freetype_Debug.lib"
Library["Freetype"] = "%{wks.location}/Nebula/Modules/FreetypeGL/Freetype/Freetype.lib"

-- Windows
Library["WinSock"] = "Ws2_32.lib"
Library["WinMM"] = "Winmm.lib"
Library["WinVersion"] = "Version.lib"
Library["BCrypt"] = "BCrypt.lib"