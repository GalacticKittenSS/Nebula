#include "nbpch.h"
#include "Renderer2D.h"

#include "Buffer.h"
#include "Vertex_Array.h"
#include "Shader.h"
#include "Render_Command.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Nebula {
	struct Renderer2DStorage {
		Ref<VertexArray>	  QuadVertexArray;
		Ref<VertexArray>  TriangleVertexArray;
		Ref<Shader>				TextureShader;
		Ref<Texture2D>			 whiteTexture;
	};

	static Renderer2DStorage* s_Data;

	void Renderer2D::Init() {
		NB_PROFILE_FUNCTION();

		s_Data = new Renderer2DStorage();
		s_Data->QuadVertexArray = VertexArray::Create();
		s_Data->TriangleVertexArray = VertexArray::Create();

		BufferLayout layout = {
			{ShaderDataType::Float3, "position"},
			{ShaderDataType::Float2, "texCoord"}
		};

		//QUAD
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

		//TRIANGLE
		float TriangleVertices[3 * (3 + 2)] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.0f,  0.5f, 0.0f, 0.5f, 1.0f
		};

		Ref<VertexBuffer> triangleVB = VertexBuffer::Create(TriangleVertices, sizeof(TriangleVertices));
		triangleVB->SetLayout(layout);
		s_Data->TriangleVertexArray->AddVertexBuffer(triangleVB);

		uint32_t triangleIndices[3] = { 0, 1, 2 };

		Ref<IndexBuffer> triangleIB = IndexBuffer::Create(triangleIndices, sizeof(triangleIndices) / sizeof(uint32_t));
		s_Data->TriangleVertexArray->SetIndexBuffer(triangleIB);

		s_Data->TextureShader = Shader::Create("assets/shaders/Default.glsl");
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

	void Renderer2D::DrawQuad(Sprite& quad, float tiling) {
		NB_PROFILE_FUNCTION();

		if (quad.texture == nullptr)
			quad.texture = s_Data->whiteTexture;
		if (quad.shader == nullptr)
			quad.shader = s_Data->TextureShader;

		quad.shader->SetFloat4("u_Colour", quad.colour);
		quad.shader->SetFloat("u_Tiling", tiling);
		quad.texture->Bind();

		quad.shader->SetMat4("u_Transform", quad.CalculateMatrix());

		s_Data->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);

		quad.texture->Unbind();
	}
	
	void Renderer2D::DrawTriangle(Sprite& tri, float tiling) {
		NB_PROFILE_FUNCTION();

		if (tri.texture == nullptr)
			tri.texture = s_Data->whiteTexture;
		if (tri.shader == nullptr)
			tri.shader = s_Data->TextureShader;

		tri.shader->SetFloat4("u_Colour", tri.colour);
		tri.shader->SetFloat("u_Tiling", tiling);
		tri.texture->Bind();

		tri.shader->SetMat4("u_Transform", tri.CalculateMatrix());

		s_Data->TriangleVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->TriangleVertexArray);

		tri.texture->Unbind();
	}
}