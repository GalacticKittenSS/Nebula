#include "nbpch.h"
#include "OpenGL_FrameBuffer.h"

#include <glad/glad.h>

namespace Nebula {

	static const uint32_t s_MaxFrameBufferSize = 8192;

	namespace Utils {
		static GLenum TextureTarget(bool multipsampled) {
			return multipsampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}

		static void CreateTextures(bool multisampled, uint32_t* outID, uint32_t count) {
			glCreateTextures(TextureTarget(multisampled), count, outID);
		}

		static void BindTexture(bool multisampled, uint32_t id) {
			glBindTexture(TextureTarget(multisampled), id);
		}

		static void AttachColourTexture(uint32_t id, int samples, GLenum internalFormat, GLenum format, uint32_t width, uint32_t height, int index) {
			bool multisampled = samples > 1;
			if (multisampled) {
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
			}
			else {
				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multisampled), id, 0);
		}

		static void AttachDepthTexture(uint32_t id, int samples, GLenum format, GLenum attachmentType, uint32_t width, uint32_t height) {
			bool multisampled = samples > 1;
			if (multisampled) {
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
			}
			else {
				glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType , TextureTarget(multisampled), id, 0);
		}

		static bool IsDepthFormat(FramebufferTextureFormat format) {
			switch (format)
			{
				case FramebufferTextureFormat::DEPTH24STENCIL8: return true;
			}
		
			return false;
		}

		static GLenum NebulaFBFormattoGL(FramebufferTextureFormat format) {
			switch (format)
			{
				case Nebula::FramebufferTextureFormat::RGBA8:	return GL_RGBA8;
				case Nebula::FramebufferTextureFormat::RED_INT: return GL_RED_INTEGER;
			}

			NB_ASSERT(false, "");
			return 0;
		}
	}

	OpenGL_FrameBuffer::OpenGL_FrameBuffer(const FrameBufferSpecification& specifications): m_Specifications(specifications) {
		for (auto spec : m_Specifications.Attachments.Attachments) {
			if (!Utils::IsDepthFormat(spec.TextureFormat))
				m_ColourAttachmentSpecs.push_back(spec);
			else
				m_DepthAttachmentSpec = spec;
		}
		
		Invalidate();
	}

	OpenGL_FrameBuffer::~OpenGL_FrameBuffer() {
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures((GLsizei)m_ColourAttachments.size(), m_ColourAttachments.data());
		glDeleteTextures(1, &m_DepthAttachment);
	}

	void OpenGL_FrameBuffer::Invalidate() {
		if (m_RendererID) {
			glDeleteFramebuffers(1, &m_RendererID);
			glDeleteTextures((GLsizei)m_ColourAttachments.size(), m_ColourAttachments.data());
			glDeleteTextures(1, &m_DepthAttachment);

			m_ColourAttachments.clear();
			m_DepthAttachment = 0;
		}

		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		bool multiSample = m_Specifications.samples > 1;

		if (m_ColourAttachmentSpecs.size()) {
			m_ColourAttachments.resize(m_ColourAttachmentSpecs.size());
			Utils::CreateTextures(multiSample, m_ColourAttachments.data(), (GLsizei)m_ColourAttachments.size());

			for (size_t i = 0; i < m_ColourAttachments.size(); i++) {
				Utils::BindTexture(multiSample, m_ColourAttachments[i]);
				switch (m_ColourAttachmentSpecs[i].TextureFormat) {
					case FramebufferTextureFormat::RGBA8:
						Utils::AttachColourTexture(m_ColourAttachments[i], m_Specifications.samples, GL_RGBA8, GL_RGBA, m_Specifications.Width, m_Specifications.Height, (int)i);
						break;
					case FramebufferTextureFormat::RED_INT:
						Utils::AttachColourTexture(m_ColourAttachments[i], m_Specifications.samples, GL_R32I, GL_RED_INTEGER, m_Specifications.Width, m_Specifications.Height, (int)i);
						break;
				}
			}
		}

		if (m_DepthAttachmentSpec.TextureFormat != FramebufferTextureFormat::None) {
			Utils::CreateTextures(multiSample, &m_DepthAttachment, 1);
			Utils::BindTexture(multiSample, m_DepthAttachment);
			switch (m_DepthAttachmentSpec.TextureFormat)
			{
			case FramebufferTextureFormat::DEPTH24STENCIL8:
				Utils::AttachDepthTexture(m_DepthAttachment, m_Specifications.samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Specifications.Width, m_Specifications.Height);
				break;
			}
		}

		if (m_ColourAttachments.size() > 1) {
			NB_ASSERT(m_ColourAttachments.size() <= 4, "");
			GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };

			glDrawBuffers((GLsizei)m_ColourAttachments.size(), buffers);
		}
		else if (m_ColourAttachments.empty()) {
			glDrawBuffer(GL_NONE);
		}

		NB_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer creation failed!")

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGL_FrameBuffer::Bind() {
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glViewport(0, 0, m_Specifications.Width, m_Specifications.Height);
	}

	void OpenGL_FrameBuffer::Unbind() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGL_FrameBuffer::Resize(uint32_t width, uint32_t height) {
		if (width == 0 || height == 0 || width > s_MaxFrameBufferSize || height > s_MaxFrameBufferSize) {
			NB_WARN("Attempted to resize framebuffer to ({0}, {1})", width, height);
			return;
		}

		m_Specifications.Width = width;
		m_Specifications.Height = height;

		Invalidate();
	}


	int OpenGL_FrameBuffer::ReadPixel(uint32_t attachmentIndex, int x, int y) {
		NB_ASSERT(attachmentIndex < m_ColourAttachments.size(), "Index is greater than Attachment Size");

		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
		int pixelData;
		glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);
		return pixelData;
	}

	void OpenGL_FrameBuffer::ClearAttachment(uint32_t attachmentIndex, int value) {
		NB_ASSERT(attachmentIndex < m_ColourAttachments.size(), "");

		auto& spec = m_ColourAttachmentSpecs[attachmentIndex];
		glClearTexImage(m_ColourAttachments[attachmentIndex], 0, 
			Utils::NebulaFBFormattoGL(spec.TextureFormat), GL_INT, &value);
	}
}