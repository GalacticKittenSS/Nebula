#include "nbpch.h"
#include "Renderer2D.h"

#include "Buffer.h"
#include "Vertex_Array.h"
#include "Shader.h"
#include "Render_Command.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Nebula {
	struct Renderer2DStorage {
		Ref<VertexArray> QuadVertexArray;
		Ref<Shader>		   TextureShader;
		Ref<Texture2D>		whiteTexture;
	};

	static Renderer2DStorage* s_Data;

	void Renderer2D::Init() {
		NB_PROFILE_FUNCTION();

		s_Data = new Renderer2DStorage();
		s_Data->QuadVertexArray = VertexArray::Create();

		BufferLayout layout = {
			{ShaderDataType::Float3, "position"},
			{ShaderDataType::Float2, "texCoord"}
		};

		float vertices[4 * (3 + 2)] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};


		Ref<VertexBuffer> squareVB = VertexBuffer::Create(vertices, sizeof(vertices));
		squareVB->SetLayout(layout);
		s_Data->QuadVertexArray->AddVertexBuffer(squareVB);

		uint32_t indices[6] = { 0, 1, 2, 2, 3, 0 };

		Ref<IndexBuffer> squareIB = IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
		s_Data->QuadVertexArray->SetIndexBuffer(squareIB);

		s_Data->TextureShader = Shader::Create("assets/shaders/Texture.glsl");
		s_Data->TextureShader->Bind();
		s_Data->TextureShader->SetInt("u_Texture", 0);

		s_Data->whiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data->whiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
	}

	void Renderer2D::Shutdown() {
		NB_PROFILE_FUNCTION();

		delete s_Data;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera) {
		NB_PROFILE_FUNCTION();

		s_Data->TextureShader->Bind();
		s_Data->TextureShader->SetMat4("u_View", camera.GetViewProjectionMatrix());
	}

	void Renderer2D::EndScene() {
		NB_PROFILE_FUNCTION();

	}

	void Renderer2D::DrawQuad(Quad& quad, float tiling) {
		NB_PROFILE_FUNCTION();

		if (quad.texture == nullptr)
			quad.texture = s_Data->whiteTexture;

		s_Data->TextureShader->SetFloat4("u_Colour", quad.colour);
		s_Data->TextureShader->SetFloat("u_Tiling", tiling);
		quad.texture->Bind();

		s_Data->TextureShader->SetMat4("u_Transform", quad.CalculateMatrix());

		s_Data->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);

		quad.texture->Unbind();
	}
}