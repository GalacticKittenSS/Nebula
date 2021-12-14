#include "nbpch.h"
#include "Renderer2D.h"

#include "Buffer.h"
#include "Vertex_Array.h"
#include "Shader.h"
#include "Render_Command.h"

namespace Nebula {
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
	};
	static Renderer2DData s_Data;

	struct RenderData {
		mat4 Transform;
		Ref<Texture2D> Texture;
		vec4 Colour;
		
		vec2* TextureCoordinates;

		RenderData(const mat4& transform, Ref<Texture2D> texture, const vec4& colour, vec2* texCoords) {
			Transform = transform;
			Texture = texture;
			Colour = colour;
			TextureCoordinates = texCoords;
		}
	};

	struct Vertex {
		vec3 Position;
		vec4 Colour;
		vec2 TexCoord;
		float TexIndex;
		float TilingFactor;
	};

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

		delete[] s_Data.QuadVBBase;
		delete[] s_Data.TriVBBase;
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
	
	void Renderer2D::BeginScene(const Camera& camera, const mat4& transform) {
		NB_PROFILE_FUNCTION();

		mat4 viewProj = camera.GetProjection() * inverse(transform);

		s_Data.TextureShader->Bind();
		s_Data.TextureShader->SetMat4("u_View", viewProj);

		s_Data.QuadIndexCount = 0;
		s_Data.QuadVBPtr = s_Data.QuadVBBase;

		s_Data.TriIndexCount = 0;
		s_Data.TriVBPtr = s_Data.TriVBBase;

		s_Data.TextureSlotIndex = 1;
	}
	
	void Renderer2D::Draw(const uint32_t type, const vec4* vertexPos, const uint32_t vertexCount, const mat4& transform, Ref<Texture2D> texture) {
		NB_PROFILE_FUNCTION();
		
		if (s_Data.TriIndexCount >= s_Data.MaxTriIndices)
			FlushAndReset();

		if (texture == nullptr)
			texture = s_Data.whiteTexture;

		if (type == NB_QUAD) {
			vec2* texCoords = new vec2[vertexCount];

			for (uint32_t i = 0; i < vertexCount; i+=4) {
				texCoords[i + 0] = { 0, 0 };
				texCoords[i + 1] = { 1, 0 };
				texCoords[i + 2] = { 1, 1 };
				texCoords[i + 3] = { 0, 1 };
			};

			s_Data.QuadVBPtr = Draw(s_Data.QuadVBPtr, vertexCount, vertexPos, { transform, texture, { 1.0f, 1.0f, 1.0f, 1.0f }, texCoords }, 1.0f);
			s_Data.QuadIndexCount += uint32_t(vertexCount * 1.5);
		} else if (type == NB_TRI) {
			vec2 texCoords[] = {
				{ 0, 0 },
				{ 1, 0 },
				{ 0.5f, 0.5f }
			};

			s_Data.TriVBPtr  = Draw(s_Data.TriVBPtr, vertexCount, vertexPos, { transform, texture,  { 1.0f, 1.0f, 1.0f, 1.0f }, texCoords }, 1.0f);
			s_Data.TriIndexCount += vertexCount;
		}
	}

	void Renderer2D::DrawQuad(Sprite& quad, float tiling) {
		NB_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= s_Data.MaxQuadIndices)
			FlushAndReset();

		vec4 vertexPos[] = { 
			{ -0.5f, -0.5f, 0.0f, 1.0f }, 
			{  0.5f, -0.5f, 0.0f, 1.0f }, 
			{  0.5f,  0.5f, 0.0f, 1.0f }, 
			{ -0.5f,  0.5f, 0.0f, 1.0f }
		};

		mat4 transform = translate(quad.position) * scale(vec3(quad.size, 1.0f));

		if (quad.rotation > 0.0f)
			transform *= rotate(quad.rotation, { 0.0f, 0.0f, 1.0f });

		Ref<Texture2D> texture = quad.texture;

		if (texture == nullptr)
			texture = s_Data.whiteTexture;

		s_Data.QuadVBPtr = Draw(s_Data.QuadVBPtr, 4, vertexPos, { transform, texture, quad.colour, quad.texCoords }, tiling);
		
		s_Data.QuadIndexCount += 6;
	}

	void Renderer2D::DrawQuad(const mat4& matrix, const vec4& colour, float tiling) {
		NB_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= s_Data.MaxQuadIndices)
			FlushAndReset();

		float textureIndex = 0.0f;

		vec4 vertexPos[4] = {
			{ -0.5f, -0.5f, 0.0f, 1.0f },
			{  0.5f, -0.5f, 0.0f, 1.0f },
			{  0.5f,  0.5f, 0.0f, 1.0f },
			{ -0.5f,  0.5f, 0.0f, 1.0f }
		};

		vec2 texCoords[4] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
		
		s_Data.QuadVBPtr = Draw(s_Data.QuadVBPtr, 4, vertexPos, { matrix, s_Data.whiteTexture, colour, texCoords }, 1.0f);

		s_Data.QuadIndexCount += 6;
	}
	
	void Renderer2D::DrawTriangle(Sprite& tri, float tiling) {
		NB_PROFILE_FUNCTION();

		if (s_Data.TriIndexCount >= s_Data.MaxTriIndices)
			FlushAndReset();

		vec4 vertexPos[] = { 
			{ -0.5f, -0.5f, 0.0f, 1.0f }, 
			{  0.5f, -0.5f, 0.0f, 1.0f }, 
			{  0.0f,  0.5f, 0.0f, 1.0f } 
		};

		mat4 transform = translate(tri.position) * scale(vec3(tri.size, 1.0f));

		if (tri.rotation > 0.0f)
			transform *= rotate(tri.rotation, { 0.0f, 0.0f, 1.0f });

		Ref<Texture2D> texture = tri.texture;

		if (texture == nullptr)
			texture = s_Data.whiteTexture;
		
		s_Data.TriVBPtr = Draw(s_Data.TriVBPtr, 4, vertexPos, { transform, texture, tri.colour, tri.texCoords }, tiling);

		s_Data.TriIndexCount += 3;
	}

	Vertex* Renderer2D::Draw(Vertex* vertexPtr, const uint32_t vertexCount, const vec4* vertexPos, const RenderData& data, float tiling) {
		NB_PROFILE_FUNCTION();

		float textureIndex = 0.0f;

		if (data.Texture != nullptr) {
			for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++) {
				if (*s_Data.TextureSlots[i].get() == *data.Texture.get()) {
					textureIndex = (float)i;
					break;
				}
			}

			if (textureIndex == 0.0f) {
				if (s_Data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
					FlushAndReset();

				textureIndex = (float)s_Data.TextureSlotIndex;
				s_Data.TextureSlots[s_Data.TextureSlotIndex] = data.Texture;
				s_Data.TextureSlotIndex++;
			}
		}

		for (size_t i = 0; i < vertexCount; i++) {
			vertexPtr->Position = data.Transform * vertexPos[i];
			vertexPtr->Colour = data.Colour;
			vertexPtr->TexCoord = data.TextureCoordinates[i];
			vertexPtr->TexIndex = textureIndex;
			vertexPtr->TilingFactor = tiling;
			vertexPtr++;
		}

		return vertexPtr;
	}

	void Renderer2D::EndScene() {
		NB_PROFILE_FUNCTION();

		uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVBPtr - (uint8_t*)s_Data.QuadVBBase);
		s_Data.QuadVertexBuffer->SetData(s_Data.QuadVBBase, dataSize);

		dataSize = (uint32_t)((uint8_t*)s_Data.TriVBPtr - (uint8_t*)s_Data.TriVBBase);
		s_Data.TriangleVertexBuffer->SetData(s_Data.TriVBBase, dataSize);

		Flush(s_Data.TriangleVertexArray, s_Data.TriIndexCount);
		Flush(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
	}

	void Renderer2D::Flush(Ref<VertexArray> vertexArray, uint32_t IndexCount) {
		NB_PROFILE_FUNCTION();

		if (IndexCount == 0)
			return;

		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
			s_Data.TextureSlots[i]->Bind(i);

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray, IndexCount);
	}

	void Renderer2D::FlushAndReset() {
		EndScene();
		
		s_Data.QuadIndexCount = 0;
		s_Data.QuadVBPtr = s_Data.QuadVBBase;

		s_Data.TriIndexCount = 0;
		s_Data.TriVBPtr = s_Data.TriVBBase;

		s_Data.TextureSlotIndex = 1;
	}
}