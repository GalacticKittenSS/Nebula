-- Nebula Dependencies

VULKAN_SDK = os.getenv("VULKAN_SDK")

includedir = {}
includedir["Box2D"] =		"%{wks.location}/Nebula/Modules/Box2D/include"
includedir["Entt"] =		"%{wks.location}/Nebula/Modules/entt/include"
includedir["GLad"] =		"%{wks.location}/Nebula/Modules/glad/include"
includedir["GLFW"] =		"%{wks.location}/Nebula/Modules/glfw/include"
includedir["ImGui"] =		"%{wks.location}/Nebula/Modules/imgui/include"
includedir["ImGuizmo"] =	"%{wks.location}/Nebula/Modules/imguizmo"
includedir["Spdlog"] =		"%{wks.location}/Nebula/Modules/spdlog/include"
includedir["Stb"] =			"%{wks.location}/Nebula/Modules/stb_image"
includedir["Yaml"] =		"%{wks.location}/Nebula/Modules/yaml-cpp/include"

includedir["shaderc"] =		"%{wks.location}/Nebula/Modules/shaderc/include"
includedir["SPIRV_Cross"] = "%{wks.location}/Nebula/Modules/SPIRV-Cross"
includedir["VulkanSDK"] = "	 %{VULKAN_SDK}/Include"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["VulkanSDK_Debug"] = "%{wks.location}/Nebula/Modules/VulkanSDK/Lib"
LibraryDir["VulkanSDK_DebugDLL"] = "%{wks.location}/Nebula/Modules/VulkanSDK/Bin"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"