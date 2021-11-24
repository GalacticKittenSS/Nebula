#pragma once

#include <string>
#include <glm/glm.hpp>

namespace Nebula {
	class Shader {
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual const std::string& GetName() const = 0;

		static Ref<Shader> Create(const std::string& path);
		static Ref<Shader> Create(const std::string& name, const std::string& vertSrc, const std::string& fragSrc);

		virtual void SetInt(const std::string& name, const int value) = 0;
		virtual void SetFloat(const std::string& name, const float value) = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec3& values) = 0;
		virtual void SetFloat4(const std::string& name, const glm::vec4& values) = 0;
	};

	class ShaderLibrary {
	public:
		void Add(const Ref<Shader>& shader);
		void Add(const std::string& name, const Ref<Shader>& shader);
		Ref<Shader> Load(const std::string& filepath);
		Ref<Shader> Load(const std::string& filepath, const std::string& name);

		Ref<Shader> Get(const std::string& name);
	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};
}#pragma once

#include <string>
#include "Nebula/Core/API.h"

namespace Nebula {
	class Texture {
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual void Unbind() const = 0;
	};

	class Texture2D : public Texture {
	public:
		static Ref<Texture2D> Create(uint32_t width, uint32_t height);
		static Ref<Texture2D> Create(const std::string& path);
	};
}#pragma once

#include "Nebula/Core/API.h"

namespace Nebula {
	enum class ShaderDataType {
		None = 0, 
		Float, Float2, Float3, Float4,
		Int, Int2, Int3, Int4,
		Mat3, Mat4,
		Bool
	};

	static uint32_t ShaderDataTypeSize(ShaderDataType type) {
		switch (type) {
			case ShaderDataType::Float:		return 4;
			case ShaderDataType::Float2:	return 4 * 2;
			case ShaderDataType::Float3:	return 4 * 3;
			case ShaderDataType::Float4:	return 4 * 4;
			case ShaderDataType::Int:		return 4;
			case ShaderDataType::Int2:		return 4 * 2;
			case ShaderDataType::Int3:		return 4 * 3;
			case ShaderDataType::Int4:		return 4 * 4;
			case ShaderDataType::Mat3:		return 4 * 4 * 3;
			case ShaderDataType::Mat4:		return 4 * 4 * 4;
			case ShaderDataType::Bool:		return 1;
		}

		NB_ASSERT(false, "Unknow Shader Data Type!");
		return 0;
	}

	struct BufferElement {
		std::string Name;
		ShaderDataType Type;
		uint32_t Offset;
		uint32_t Size;
		bool Normalized;

		BufferElement() = default;
		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false): 
			Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized) { }

		uint32_t GetComponentCount() const { 
			switch (Type) {
			case ShaderDataType::Bool:		return 1;
			case ShaderDataType::Int:		return 1;
			case ShaderDataType::Float:		return 1;
			case ShaderDataType::Int2:		return 2;
			case ShaderDataType::Float2:	return 2;
			case ShaderDataType::Int3:		return 3;
			case ShaderDataType::Float3:	return 3;
			case ShaderDataType::Int4:		return 4;
			case ShaderDataType::Float4:	return 4;
			case ShaderDataType::Mat3:		return 3 * 3;
			case ShaderDataType::Mat4:		return 4 * 4;
			}

			NB_ASSERT(false, "Unknow Shader Data Type!");
			return 0;
		}
	};

	class BufferLayout {
	public:
		BufferLayout() { }
		BufferLayout(const std::initializer_list<BufferElement> elements): m_Elements(elements) {
			CalculateOffsetsAndStride();
		}

		inline uint32_t GetStride() const { return m_Stride; }
		inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }
	private:
		void CalculateOffsetsAndStride() {
			uint32_t offset = 0;
			m_Stride = 0;

			for (auto& element : m_Elements) {
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}
	private:
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride = 0;
	};

	class VertexBuffer {
	public:
		virtual ~VertexBuffer() { }

		virtual void Bind()   const = 0;
		virtual void Unbind() const = 0;

		virtual void SetLayout(const BufferLayout& layout) = 0;
		virtual const BufferLayout GetLayout() const = 0;

		//static Ref<VertexBuffer> Create(uint32_t size);
		static Ref<VertexBuffer> Create(float* vertices, uint32_t size);
	};

	//Only Supports 32-bit index buffers;
	class IndexBuffer {
	public:
		virtual ~IndexBuffer() { }

		virtual void Bind()   const = 0;
		virtual void Unbind() const = 0;

		virtual uint32_t GetCount() const = 0;

		static Ref<IndexBuffer> Create(uint32_t* indices, uint32_t count);
	};
}#pragma once

#include <memory>
#include "Buffer.h"

namespace Nebula {
	class VertexArray {
	public:
		virtual ~VertexArray() { }

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void AddVertexBuffer(const Ref<VertexBuffer>& buffer) = 0;
		virtual void SetIndexBuffer(const Ref<IndexBuffer>& buffer) = 0;

		virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const = 0;
		virtual const Ref<IndexBuffer>& GetIndexBuffer() const = 0;

		static Ref<VertexArray> Create();
	};
}#pragma once

#include <glm/glm.hpp>

namespace Nebula {
	class OrthographicCamera {
	public:
		OrthographicCamera(float left, float right, float bottom, float top);

		void SetProjection(float left, float right, float bottom, float top);

		const glm::vec3& GetPosition() { return m_Position; }
		void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); }

		const float& GetRotation() { return m_Rotation; }
		void SetRotation(const float& rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }

		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }
	private:
		void RecalculateViewMatrix();
	private:
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ViewProjectionMatrix;

		glm::vec3 m_Position = { 0, 0, 0 };
		float m_Rotation = 0.0f;
	};
}#pragma once

#include "Camera.h"

#include "Nebula/core/Time.h"

#include "Nebula/events/Event.h"
#include "Nebula/events/Window_Event.h"
#include "Nebula/events/Mouse_Event.h"


namespace Nebula {
	class OrthographicCameraController {
	public:
		OrthographicCameraController(float aspectRatio = 16.0f / 9.0f, bool rotation = false);

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

		OrthographicCamera& GetCamera() { return m_Camera; }
		const OrthographicCamera& GetCamera() const { return m_Camera; }

		void SetZoomLevel(float level) { m_ZoomLevel = level; }
		float GetZoomLevel() const { return m_ZoomLevel; }
	private:
		bool OnMouseScrolled(MouseScrolledEvent& event);
		bool OnWindowResized(WindowResizeEvent& event);
	private:
		float m_AspectRatio;
		float m_ZoomLevel = 1.0f;
		OrthographicCamera m_Camera;

		bool m_Rotation;

		glm::vec3 m_CamPosition = { 0.0f, 0.0f, 0.0f };
		float m_CamRotation = 0.0f;

		float m_CamTranslationSpeed = 5.0f, m_CamRotationSpeed = 180.0f;
	};
}#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Texture.h"
#include "Shader.h"

namespace Nebula {
	struct Sprite{
		Sprite(): position({ 0, 0, 0 }), size({ 1, 1 }) { }
		
		Sprite(glm::vec2 pos, glm::vec2 size, float rotation = 0.0f):
			position({ pos.x, pos.y, 0.0f }), size(size), rotation(glm::radians(rotation)) { }
		
		Sprite(glm::vec3 pos, glm::vec2 size, float rotation = 0.0f):
			position(pos), size(size), rotation(glm::radians(rotation)) { }

		Sprite(glm::vec2 pos, glm::vec2 size, float rotation, glm::vec4 colour, Ref<Texture2D> texture = nullptr):
			position({ pos.x, pos.y, 0.0f }), size(size), rotation(glm::radians(rotation)), colour(colour), texture(texture) { }
		
		Sprite(glm::vec3 pos, glm::vec2 size, float rotation, glm::vec4 colour, Ref<Texture2D> texture = nullptr):
			position(pos), size(size), rotation(glm::radians(rotation)), colour(colour), texture(texture) { }

		Sprite(glm::vec2 pos, glm::vec2 size, float rotation, Ref<Texture2D> texture) :
			position({ pos.x, pos.y, 0.0f }), size(size), rotation(glm::radians(rotation)), texture(texture) { }

		Sprite(glm::vec3 pos, glm::vec2 size, float rotation, Ref<Texture2D> texture) :
			position(pos), size(size), rotation(glm::radians(rotation)), texture(texture) { }

		glm::vec3 position;
		glm::vec2 size;
		float rotation = 0.0f;

		Ref<Texture2D> texture = nullptr;
		Ref<Shader> shader = nullptr;
		glm::vec4 colour = { 1, 1, 1, 1 };

		//Should only be used by Renderer
		glm::mat4 CalculateMatrix() {
			glm::mat4 transform = glm::translate(glm::mat4(1.0f), this->position)
				* glm::scale(glm::mat4(1.0f), { this->size.x, this->size.y, 0.0f });
			
			if (rotation != 0.0f)
				transform *= glm::rotate(glm::mat4(1.0f), this->rotation, { 0.0f, 0.0f, 1.0f });

			return transform;
		}
	};
}#pragma once

#include <glm/glm.hpp>

#include "Vertex_Array.h"

namespace Nebula {
	class RendererAPI {
	public:
		enum class API {
			None = 0, OpenGL = 1
		};
	public:
		virtual void Init() = 0;
		virtual void SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

		virtual void Clear() = 0;
		virtual void SetClearColour(float r, float g, float b, float a) = 0;
		virtual void SetClearColour(const glm::vec4& colour) = 0;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray) = 0;

		inline static API GetAPI() { return s_API; }
		static Scope<RendererAPI> Create();
	private:
		static API s_API;
	};
}#pragma once

#include "Renderer_API.h"

namespace Nebula {
	class RenderCommand {
	public:
		inline static void Init() {
			s_RendererAPI->Init();
		}

		inline static void SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
			s_RendererAPI->SetViewPort(x ,y, width, height);
		}

		inline static void Clear() {
			s_RendererAPI->Clear();
		}

		inline static void SetClearColour(float r, float g, float b, float a) {
			s_RendererAPI->SetClearColour(r, g, b, a);
		}

		inline static void SetClearColour(const glm::vec4& colour) {
			s_RendererAPI->SetClearColour(colour);
		}

		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray) {
			s_RendererAPI->DrawIndexed(vertexArray);
		}
	private:
		static Scope<RendererAPI> s_RendererAPI;
	};
}#pragma once

#include "Render_Command.h"
#include "Camera.h"
#include "Shader.h"

namespace Nebula {
	class Renderer {
	public:
		static void Init();
		static void Shutdown();

		static void OnWindowResize(uint32_t width, uint32_t height);

		static void BeginScene(OrthographicCamera& camera);
		static void EndScene();

		static void Submit(
			const Ref<Shader>& shader,
			const Ref<VertexArray>& vertexArray,
			const glm::mat4& transform = glm::mat4(1.0f)
		);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	private:
		struct SceneData {
			glm::mat4 ViewProjectMatrix;
		};

		static Scope<SceneData> s_Data;
	};
}#pragma once

#include "Camera.h"
#include "Texture.h"
#include "GameObjects.h"

namespace Nebula {
	class Renderer2D {
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();

		//Primitives
		static void DrawQuad(Sprite& quad, float tiling = 1.0f);
		static void DrawTriangle(Sprite& tri, float tiling = 1.0f);
	};
}#pragma once

namespace Nebula {
	class GraphicsContext {
	public:
		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;

		static Scope<GraphicsContext> Create(void* window);
	};
}