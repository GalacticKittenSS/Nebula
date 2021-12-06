#include "nbpch.h"
#include "Renderer2D.h"

#include "Buffer.h"
#include "Vertex_Array.h"
#include "Shader.h"
#include "Render_Command.h"

namespace Nebula {
	struct Vertex {
		vec3 Position;
		vec4 Colour;
		vec2 TexCoord;
		float TexIndex;
		float TilingFactor;
	};

	struct Renderer2DData {
		static const uint32_t MaxSprites = 10000;
		static const uint32_t MaxTextureSlots = 32;
		
		Ref<Shader>				 TextureShader;
		Ref<Texture2D>			  whiteTexture;

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1;

		//Quad
		static const uint32_t MaxQuadVertices  = MaxSprites * 4;
		static const uint32_t MaxQuadIndices   = MaxSprites * 6;
		
		Ref<VertexArray>	   QuadVertexArray;
		Ref<VertexBuffer>	  QuadVertexBuffer;
		
		uint32_t QuadIndexCount = 0;
		
		Vertex* QuadVBBase = nullptr;
		Vertex* QuadVBPtr  = nullptr;

		//Tri
		static const uint32_t MaxTriVertices = MaxSprites * 3;
		static const uint32_t MaxTriIndices  = MaxSprites * 3;

		Ref<VertexArray>   TriangleVertexArray;
		Ref<VertexBuffer> TriangleVertexBuffer;

		uint32_t TriIndexCount = 0;
		
		Vertex* TriVBBase = nullptr;
		Vertex* TriVBPtr = nullptr;

		Renderer2D::stats Stats;
	};

	static Renderer2DData s_Data;

	void Renderer2D::Init() {
		NB_PROFILE_FUNCTION();


		BufferLayout layout = {
			{ShaderDataType::Float3, "position"},
			{ShaderDataType::Float4, "colour"},
			{ShaderDataType::Float2, "texCoord"},
			{ShaderDataType::Float, "texIndex"},
			{ShaderDataType::Float, "tilingFactor"}
		};

		//QUAD
		s_Data.QuadVertexArray = VertexArray::Create();
		s_Data.QuadVertexBuffer = VertexBuffer::Create(s_Data.MaxQuadVertices * sizeof(Vertex));
		s_Data.QuadVertexBuffer->SetLayout(layout);
		s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);

		s_Data.QuadVBBase = new Vertex[s_Data.MaxQuadVertices];

		uint32_t* quadIndices = new uint32_t[s_Data.MaxQuadIndices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_Data.MaxQuadIndices; i += 6) {
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}

		Ref<IndexBuffer> squareIB = IndexBuffer::Create(quadIndices, s_Data.MaxQuadIndices);
		s_Data.QuadVertexArray->SetIndexBuffer(squareIB);
		delete[] quadIndices;

		//TRIANGLE
		s_Data.TriangleVertexArray = VertexArray::Create();
		s_Data.TriangleVertexBuffer = VertexBuffer::Create(s_Data.MaxTriVertices * sizeof(Vertex));
		s_Data.TriangleVertexBuffer->SetLayout(layout);
		s_Data.TriangleVertexArray->AddVertexBuffer(s_Data.TriangleVertexBuffer);
		
		s_Data.TriVBBase = new Vertex[s_Data.MaxTriVertices];
		
		uint32_t* triIndices = new uint32_t[s_Data.MaxTriIndices];
		
		offset = 0;
		for (uint32_t i = 0; i < s_Data.MaxTriIndices; i += 3) {
			triIndices[i + 0] = offset + 0;
			triIndices[i + 1] = offset + 1;
			triIndices[i + 2] = offset + 2;
		
			offset += 3;
		}
		 
		Ref<IndexBuffer> triangleIB = IndexBuffer::Create(triIndices, s_Data.MaxTriIndices);
		s_Data.TriangleVertexArray->SetIndexBuffer(triangleIB);
		delete[] triIndices;

		//Shaders
		s_Data.TextureShader = Shader::Create("assets/shaders/Default.glsl");

		//White Texture
		s_Data.whiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data.whiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
		s_Data.TextureSlots[0] = s_Data.whiteTexture;

		int32_t samplers[s_Data.MaxTextureSlots];
		for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
			samplers[i] = i;

		s_Data.TextureShader->Bind();
		s_Data.TextureShader->SetIntArray("u_Textures", samplers, s_Data.MaxTextureSlots);
	}

	void Renderer2D::Shutdown() {
		NB_PROFILE_FUNCTION();
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera) {
		NB_PROFILE_FUNCTION();

		s_Data.TextureShader->Bind();
		s_Data.TextureShader->SetMat4("u_View", camera.GetViewProjectionMatrix());

		s_Data.QuadIndexCount = 0;
		s_Data.QuadVBPtr = s_Data.QuadVBBase;

		s_Data.TriIndexCount = 0;
		s_Data.TriVBPtr = s_Data.TriVBBase;

		s_Data.TextureSlotIndex = 1;
	}

	void Renderer2D::FlushAndReset() {
		EndScene();
		
		s_Data.QuadIndexCount = 0;
		s_Data.QuadVBPtr = s_Data.QuadVBBase;

		s_Data.TriIndexCount = 0;
		s_Data.TriVBPtr = s_Data.TriVBBase;

		s_Data.TextureSlotIndex = 1;
	}

	void Renderer2D::EndScene() {
		NB_PROFILE_FUNCTION();

		uint32_t dataSize = (uint8_t*)s_Data.QuadVBPtr - (uint8_t*)s_Data.QuadVBBase;
		s_Data.QuadVertexBuffer->SetData(s_Data.QuadVBBase, dataSize);

		dataSize = (uint8_t*)s_Data.TriVBPtr - (uint8_t*)s_Data.TriVBBase;
		s_Data.TriangleVertexBuffer->SetData(s_Data.TriVBBase, dataSize);

		Flush();
	}

	void Renderer2D::Flush() {
		NB_PROFILE_FUNCTION();

		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
			s_Data.TextureSlots[i]->Bind(i);

		s_Data.QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);

		s_Data.TriangleVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data.TriangleVertexArray, s_Data.TriIndexCount);

		s_Data.Stats.DrawCalls++;
	}

	void Renderer2D::DrawQuad(Sprite& quad, float tiling) {
		NB_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= s_Data.MaxQuadIndices)
			FlushAndReset();

		vec4 colour = quad.colour;

		float textureIndex = 0.0f;
		
		if (quad.texture != nullptr) {
			for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++) {
				if (*s_Data.TextureSlots[i].get() == *quad.texture.get()) {
					textureIndex = (float)i;
					break;
				}
			}

			if (textureIndex == 0.0f) {
				textureIndex = (float)s_Data.TextureSlotIndex;
				s_Data.TextureSlots[s_Data.TextureSlotIndex] = quad.texture;
				s_Data.TextureSlotIndex++;
			}
		}

		mat4 transform = quad.CalculateMatrix();

		s_Data.QuadVBPtr->Position = transform * vec4(-0.5f, -0.5f, 0.0f, 1.0f);
		s_Data.QuadVBPtr->Colour = colour;
		s_Data.QuadVBPtr->TexCoord = { 0.0f, 0.0f };
		s_Data.QuadVBPtr->TexIndex = textureIndex;
		s_Data.QuadVBPtr->TilingFactor = tiling;
		s_Data.QuadVBPtr++;

		s_Data.QuadVBPtr->Position = transform * vec4(0.5f, -0.5f, 0.0f, 1.0f);
		s_Data.QuadVBPtr->Colour = colour;
		s_Data.QuadVBPtr->TexCoord = { 1.0f, 0.0f };
		s_Data.QuadVBPtr->TexIndex = textureIndex;
		s_Data.QuadVBPtr->TilingFactor = tiling;
		s_Data.QuadVBPtr++;

		s_Data.QuadVBPtr->Position = transform * vec4(0.5f, 0.5f, 0.0f, 1.0f);
		s_Data.QuadVBPtr->Colour = colour;
		s_Data.QuadVBPtr->TexCoord = { 1.0f, 1.0f };
		s_Data.QuadVBPtr->TexIndex = textureIndex;
		s_Data.QuadVBPtr->TilingFactor = tiling;
		s_Data.QuadVBPtr++;

		s_Data.QuadVBPtr->Position = transform * vec4(-0.5f, 0.5f, 0.0f, 1.0f);
		s_Data.QuadVBPtr->Colour = colour;
		s_Data.QuadVBPtr->TexCoord = { 0.0f, 1.0f };
		s_Data.QuadVBPtr->TexIndex = textureIndex;
		s_Data.QuadVBPtr->TilingFactor = tiling;
		s_Data.QuadVBPtr++;

		s_Data.QuadIndexCount += 6;

		s_Data.Stats.QuadCount++;
	}
	
	void Renderer2D::DrawTriangle(Sprite& tri, float tiling) {
		NB_PROFILE_FUNCTION();
		
		if (s_Data.TriIndexCount >= s_Data.MaxTriIndices)
			FlushAndReset();

		vec4 colour	  = tri.colour;

		float textureIndex = 0.0f;

		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++) {
			if (*s_Data.TextureSlots[i].get() == *tri.texture.get()) {
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f && tri.texture != nullptr) {
			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = tri.texture;
			s_Data.TextureSlotIndex++;
		}

		mat4 transform = tri.CalculateMatrix();

		s_Data.TriVBPtr->Position = transform * vec4(-0.5f, -0.5f, 0.0f, 1.0f);
		s_Data.TriVBPtr->Colour = colour;
		s_Data.TriVBPtr->TexCoord = { 0.0f, 0.0f };
		s_Data.TriVBPtr->TexIndex = textureIndex;
		s_Data.TriVBPtr->TilingFactor = tiling;
		s_Data.TriVBPtr++;

		s_Data.TriVBPtr->Position = transform * vec4(0.5f, -0.5f, 0.0f, 1.0f);
		s_Data.TriVBPtr->Colour = colour;
		s_Data.TriVBPtr->TexCoord = { 1.0f, 0.0f };
		s_Data.TriVBPtr->TexIndex = textureIndex;
		s_Data.TriVBPtr->TilingFactor = tiling;
		s_Data.TriVBPtr++;

		s_Data.TriVBPtr->Position = transform * vec4(0.0f, 0.5f, 0.0f, 1.0f);
		s_Data.TriVBPtr->Colour = colour;
		s_Data.TriVBPtr->TexCoord = { 0.5f, 0.5f };
		s_Data.TriVBPtr->TexIndex = textureIndex;
		s_Data.TriVBPtr->TilingFactor = tiling;
		s_Data.TriVBPtr++;

		s_Data.TriIndexCount += 3;

		s_Data.Stats.TriCount++;
	}

	Renderer2D::stats Renderer2D::GetStats() {
		return s_Data.Stats;
	}

	void Renderer2D::ResetStats() {
		memset(&s_Data.Stats, 0, sizeof(stats));
	}
}