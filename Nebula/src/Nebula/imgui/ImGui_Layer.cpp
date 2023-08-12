#include "nbpch.h"
#include "ImGui_Layer.h"

#include "Nebula/Core/Application.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_vulkan.h>

#include <GLFW/glfw3.h>

#include "Nebula/Renderer/Render_Command.h"
#include "Platform/Vulkan/VulkanAPI.h"
#include "Platform/Vulkan/Vulkan_Context.h"

void ImGui_ImplVulkanH_CreateWindowCommandBuffers(VkPhysicalDevice physical_device, VkDevice device, ImGui_ImplVulkanH_Window* wd, uint32_t queue_family, const VkAllocationCallbacks* allocator);

namespace Nebula {
	static ImGui_ImplVulkanH_Window vulkan_window = {};
	
	static void CreateOrResizeWindow(VkDevice device, ImGui_ImplVulkanH_Window* wd, VkSwapchainKHR swapchain, uint32_t width, uint32_t height,
		const std::vector<VkImageView>& imageViews)
	{
		wd->Swapchain = swapchain;
		wd->Width = width;
		wd->Height = height;

		for (uint32_t i = 0; i < wd->ImageCount; i++)
			wd->Frames[i].BackbufferView = imageViews[i];
		
		{
			VkImageView attachment[1];
			VkFramebufferCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			info.renderPass = wd->RenderPass;
			info.attachmentCount = 1;
			info.pAttachments = attachment;
			info.width = wd->Width;
			info.height = wd->Height;
			info.layers = 1;
			for (uint32_t i = 0; i < wd->ImageCount; i++)
			{
				ImGui_ImplVulkanH_Frame* fd = &wd->Frames[i];
				attachment[0] = fd->BackbufferView;
				vkCreateFramebuffer(device, &info, nullptr, &fd->Framebuffer);
			}
		}
	}

	ImGuiLayer::ImGuiLayer(): Layer("ImGuiLayer") { }

	ImGuiLayer::~ImGuiLayer() { }

	void ImGuiLayer::Attach() {
		NB_PROFILE_FUNCTION();

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

		io.Fonts->AddFontFromFileTTF("Resources/fonts/OpenSans/Bold.ttf", 18.0f);
		io.FontDefault = io.Fonts->AddFontFromFileTTF("Resources/fonts/OpenSans/Regular.ttf", 18.0f);

		SetDarkThemeColour();
		
		ImGui::GetStyle().FrameRounding = 4.0f;

		Application& app = Application::Get();
		Window& win = app.GetWindow();
		GLFWwindow* window = static_cast<GLFWwindow*>(win.GetNativeWindow());

		// Setup Platform/Renderer bindings
		
		if (RendererAPI::GetAPI() == RendererAPI::API::OpenGL)
		{
			ImGui_ImplGlfw_InitForOpenGL(window, true);
			ImGui_ImplOpenGL3_Init("#version 410");
		}
		else if (RendererAPI::GetAPI() == RendererAPI::API::Vulkan)
		{
			ImGui_ImplVulkan_InitInfo init_info = {};
			init_info.Instance = VulkanAPI::GetInstance();
			init_info.PhysicalDevice = VulkanAPI::GetPhysicalDevice();
			init_info.Device = VulkanAPI::GetDevice();
			init_info.QueueFamily = VulkanAPI::GetQueueFamily();
			init_info.Queue = VulkanAPI::GetQueue();
			init_info.DescriptorPool = VulkanAPI::s_DescriptorPool;
			init_info.Subpass = 0;
			init_info.MinImageCount = 3;
			init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
			init_info.ImageCount = 3;

			Vulkan_Context* context = (Vulkan_Context*)win.GetContext();
			ImGui_ImplVulkanH_Window* wd = &vulkan_window;
			wd->Surface = context->m_Surface;

			// Select Surface Format
			const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
			const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
			wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(init_info.PhysicalDevice, wd->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

			// Select Present Mode
#ifdef IMGUI_UNLIMITED_FRAME_RATE
			VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
#else
			VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
#endif
			wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(init_info.PhysicalDevice, wd->Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));
			wd->ImageCount = ImGui_ImplVulkanH_GetMinImageCountFromPresentMode(wd->PresentMode);
			wd->ClearEnable = true;

			wd->Frames = (ImGui_ImplVulkanH_Frame*)IM_ALLOC(sizeof(ImGui_ImplVulkanH_Frame) * wd->ImageCount);
			wd->FrameSemaphores = (ImGui_ImplVulkanH_FrameSemaphores*)IM_ALLOC(sizeof(ImGui_ImplVulkanH_FrameSemaphores) * wd->ImageCount);
			memset(wd->Frames, 0, sizeof(wd->Frames[0]) * wd->ImageCount);
			memset(wd->FrameSemaphores, 0, sizeof(wd->FrameSemaphores[0]) * wd->ImageCount);

			// Create SwapChain, RenderPass, Framebuffer, etc.
			//ImGui_ImplVulkanH_CreateOrResizeWindow(init_info.Instance, init_info.PhysicalDevice, init_info.Device, wd, init_info.QueueFamily, nullptr, win.GetWidth(), win.GetHeight(), 3);
			ImGui_ImplVulkanH_CreateWindowCommandBuffers(init_info.PhysicalDevice, init_info.Device, wd, init_info.QueueFamily, nullptr);
			
			// Create the Render Pass
			{
				VkAttachmentDescription attachment = {};
				attachment.format = wd->SurfaceFormat.format;
				attachment.samples = VK_SAMPLE_COUNT_1_BIT;
				attachment.loadOp = wd->ClearEnable ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				VkAttachmentReference color_attachment = {};
				color_attachment.attachment = 0;
				color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				VkSubpassDescription subpass = {};
				subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				subpass.colorAttachmentCount = 1;
				subpass.pColorAttachments = &color_attachment;
				VkSubpassDependency dependency = {};
				dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
				dependency.dstSubpass = 0;
				dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependency.srcAccessMask = 0;
				dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				VkRenderPassCreateInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
				info.attachmentCount = 1;
				info.pAttachments = &attachment;
				info.subpassCount = 1;
				info.pSubpasses = &subpass;
				info.dependencyCount = 1;
				info.pDependencies = &dependency;
				vkCreateRenderPass(init_info.Device, &info, nullptr, &wd->RenderPass);
			}

			CreateOrResizeWindow(init_info.Device, wd, context->m_SwapChain, context->m_Extent.width, context->m_Extent.height, context->m_ImageViews);

			ImGui_ImplGlfw_InitForVulkan(window, true);
			ImGui_ImplVulkan_Init(&init_info, vulkan_window.RenderPass);

			// Upload Fonts
			{
				// Use any command queue
				VkCommandPool command_pool = wd->Frames[wd->FrameIndex].CommandPool;
				VkCommandBuffer command_buffer = wd->Frames[wd->FrameIndex].CommandBuffer;

				vkResetCommandPool(init_info.Device, command_pool, 0);
				VkCommandBufferBeginInfo begin_info = {};
				begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
				vkBeginCommandBuffer(command_buffer, &begin_info);
				
				ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

				VkSubmitInfo end_info = {};
				end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				end_info.commandBufferCount = 1;
				end_info.pCommandBuffers = &command_buffer;
				vkEndCommandBuffer(command_buffer);
				vkQueueSubmit(init_info.Queue, 1, &end_info, VK_NULL_HANDLE);
				
				vkDeviceWaitIdle(init_info.Device);
				ImGui_ImplVulkan_DestroyFontUploadObjects();
			}
		}
	}

	void ImGuiLayer::Detach() {
		NB_PROFILE_FUNCTION();

		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL:
			ImGui_ImplOpenGL3_Shutdown();
			break;
		case RendererAPI::API::Vulkan:
			ImGui_ImplVulkan_Shutdown();
			break;
		}
			
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::OnEvent(Event& e) {
		if (m_BlockEvents) {
			ImGuiIO& io = ImGui::GetIO();
			e.Handled |= e.IsInCategory(MouseCat) & io.WantCaptureMouse;
			e.Handled |= e.IsInCategory(KeyboardCat) & io.WantCaptureKeyboard;
		}
	}

	void ImGuiLayer::Begin() {
		NB_PROFILE_FUNCTION();

		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL:
			ImGui_ImplOpenGL3_NewFrame();
			break;
		case RendererAPI::API::Vulkan:
			ImGui_ImplVulkan_NewFrame();
			break;
		}

		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void ImGuiLayer::End() {
		NB_PROFILE_FUNCTION();

		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		// Rendering
		ImGui::Render();

		if (RendererAPI::GetAPI() == RendererAPI::API::OpenGL)
		{
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}
		else if (RendererAPI::GetAPI() == RendererAPI::API::Vulkan)
		{
			Vulkan_Context* context = (Vulkan_Context*)Application::Get().GetWindow().GetContext();
			if (context->m_SwapChain != vulkan_window.Swapchain)
			{
				// Swapchain has been updated or destroyed!
				CreateOrResizeWindow(VulkanAPI::GetDevice(), &vulkan_window, context->m_SwapChain, context->m_Extent.width, context->m_Extent.height,
					context->m_ImageViews);
			}

			ImGui_ImplVulkanH_Window* wd = &vulkan_window;
			wd->SemaphoreIndex = VulkanAPI::s_FrameIndex;
			wd->FrameIndex = context->m_ImageIndex;
			
			ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];

			{
				vkWaitForFences(VulkanAPI::GetDevice(), 1, &fd->Fence, VK_TRUE, UINT64_MAX);
				vkResetFences(VulkanAPI::GetDevice(), 1, &fd->Fence);
				vkResetCommandPool(VulkanAPI::GetDevice(), fd->CommandPool, 0);

				VkCommandBufferBeginInfo beginInfo{};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

				VkResult result = vkBeginCommandBuffer(fd->CommandBuffer, &beginInfo);
				NB_ASSERT(result == VK_SUCCESS, "Failed to begin recording command buffer!");
			}
			
			// Begin Render Pass
			{
				VkRenderPassBeginInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				info.renderPass = wd->RenderPass;
				info.framebuffer = fd->Framebuffer;
				info.renderArea.extent.width = wd->Width;
				info.renderArea.extent.height = wd->Height;
				info.clearValueCount = 1;
				info.pClearValues = &wd->ClearValue;
				vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
			}

			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), fd->CommandBuffer);

			// Submit command buffer
			vkCmdEndRenderPass(fd->CommandBuffer);
			{
				VkResult result = vkEndCommandBuffer(fd->CommandBuffer);
				NB_ASSERT(result == VK_SUCCESS, "Failed to record command buffer!");

				VkSubmitInfo submitInfo{};
				submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

				VkSemaphore waitSemaphores[] = { VulkanAPI::GetRenderSemaphore() };
				VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
				submitInfo.waitSemaphoreCount = 1;
				submitInfo.pWaitSemaphores = waitSemaphores;
				submitInfo.pWaitDstStageMask = waitStages;
				submitInfo.commandBufferCount = 1;
				submitInfo.pCommandBuffers = &fd->CommandBuffer;

				VkSemaphore signalSemaphores[] = { VulkanAPI::GetRenderSemaphore() };
				submitInfo.signalSemaphoreCount = 1;
				submitInfo.pSignalSemaphores = signalSemaphores;

				result = vkQueueSubmit(VulkanAPI::GetQueue(), 1, &submitInfo, fd->Fence);
				NB_ASSERT(result == VK_SUCCESS, "Failed to submit draw command buffer!");
			}
		}

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}

	void ImGuiLayer::SetColours(ImVec4 primary, ImVec4 text, ImVec4 regular, ImVec4 hovered, ImVec4 active) {
		auto& colours = ImGui::GetStyle().Colors;
		colours[ImGuiCol_WindowBg] = primary;
		colours[ImGuiCol_MenuBarBg] = active;
		colours[ImGuiCol_PopupBg] = primary;
		colours[ImGuiCol_Text] = text;

		// Headers
		colours[ImGuiCol_Header] = active;
		colours[ImGuiCol_HeaderHovered] = hovered;
		colours[ImGuiCol_HeaderActive] = regular;

		// Buttons
		colours[ImGuiCol_Button] = regular;
		colours[ImGuiCol_ButtonHovered] = hovered;
		colours[ImGuiCol_ButtonActive] = active;

		// Frame BG
		colours[ImGuiCol_FrameBg] = regular;
		colours[ImGuiCol_FrameBgHovered] = hovered;
		colours[ImGuiCol_FrameBgActive] = active;

		// Tabs
		colours[ImGuiCol_Tab] = regular;
		colours[ImGuiCol_TabHovered] = hovered; 
		colours[ImGuiCol_TabActive] = hovered;
		colours[ImGuiCol_TabUnfocused] = regular;
		colours[ImGuiCol_TabUnfocusedActive] = primary;

		// Title
		colours[ImGuiCol_TitleBg] = active;
		colours[ImGuiCol_TitleBgActive] = active;
		colours[ImGuiCol_TitleBgCollapsed] = active;
	}

	void ImGuiLayer::SetDarkThemeColour() {
		ImVec4 primary = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		ImVec4 text = ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f };
		ImVec4 regular = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };
		ImVec4 hovered = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		ImVec4 active  = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		SetColours(primary, text, regular, hovered, active);
	}

	void ImGuiLayer::SetColdThemeColour() {
		ImVec4 primary = ImVec4{ 0.175f, 0.2f, 0.26f, 1.0f };
		ImVec4 text = ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f };
		ImVec4 regular = ImVec4{ 0.075f, 0.105f, 0.16f, 1.0f };
		ImVec4 hovered = ImVec4{ 0.275f, 0.305f, 0.36f, 1.0f };
		ImVec4 active = ImVec4{ 0.125f, 0.1505f, 0.201f, 1.0f };

		SetColours(primary, text, regular, hovered, active);
	}

	void ImGuiLayer::SetLightThemeColour() {
		ImVec4 primary = ImVec4{ 0.8f, 0.805f, 0.81f, 1.0f };
		ImVec4 text = ImVec4{ 0.115f, 0.12f, 0.125f, 1.0f };
		ImVec4 regular = ImVec4{ 0.5f, 0.505f, 0.51f, 1.0f };
		ImVec4 hovered = ImVec4{ 0.9f, 0.905f, 0.91f, 1.0f };
		ImVec4 active = ImVec4{ 0.55f, 0.5505f, 0.551f, 1.0f };

		SetColours(primary, text, regular, hovered, active);
	}

	void ImGuiLayer::SetWarmThemeColour() {
		ImVec4 primary = ImVec4{ 0.9f, 0.755f, 0.61f, 1.0f };
		ImVec4 text = ImVec4{ 0.115f, 0.12f, 0.125f, 1.0f };
		ImVec4 regular = ImVec4{ 0.7f, 0.505f, 0.41f, 1.0f };
		ImVec4 hovered = ImVec4{ 1.0f, 0.905f, 0.61f, 1.0f };
		ImVec4 active = ImVec4{ 0.65f, 0.4505f, 0.351f, 1.0f };

		SetColours(primary, text, regular, hovered, active);
	}

	uint32_t ImGuiLayer::GetActiveWidgetID() const
	{
		return GImGui->ActiveId;
	}
}