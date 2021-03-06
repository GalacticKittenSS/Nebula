-- Nebula Dependencies

VULKAN_SDK = os.getenv("VULKAN_SDK")

includedir = {}
includedir["Nebula"] =		"%{wks.location}/Nebula/include"
includedir["Box2D"] =		"%{wks.location}/Nebula/Modules/Box2D/include"
includedir["Entt"] =		"%{wks.location}/Nebula/Modules/entt/include"
includedir["GLad"] =		"%{wks.location}/Nebula/Modules/glad/include"
includedir["GLFW"] =		"%{wks.location}/Nebula/Modules/glfw/include"
includedir["ImGui"] =		"%{wks.location}/Nebula/Modules/imgui/include"
includedir["ImGuizmo"] =	"%{wks.location}/Nebula/Modules/imguizmo"
includedir["Spdlog"] =		"%{wks.location}/Nebula/Modules/spdlog/include"
includedir["Stb"] =			"%{wks.location}/Nebula/Modules/stb_image"
includedir["Yaml"] =		"%{wks.location}/Nebula/Modules/yaml-cpp/include"
includedir["FreetypeGL"] =	"%{wks.location}/Nebula/Modules/FreetypeGL/include"

includedir["shaderc"] =		"%{wks.location}/Nebula/Modules/shaderc/include"
includedir["SPIRV_Cross"] = "%{wks.location}/Nebula/Modules/SPIRV-Cross"
includedir["VulkanSDK"] = "	 %{VULKAN_SDK}/Include"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"

Library = {}
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