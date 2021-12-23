#include "nbpch.h"
#include "Renderer2D.h"

#include "Buffer.h"
#include "Vertex_Array.h"
#include "Shader.h"
#include "Render_Command.h"
#include "UniformBuffer.h"

#include "Nebula/Scene/Components.h"

namespace Nebula {
	struct Renderer2DData {
		static const uint32_t MaxSprites = 10000;
		static const uint32_t MaxTextureSlots = 32;
		static const uint32_t MaxVertices  = MaxSprites * 4;
		static const uint32_t MaxIndices   = MaxSprites * 6;
		
		Ref<Shader>		TextureShader;
		Ref<Shader>		 CircleShader;
		Ref<Shader>		   LineShader;
		Ref<Texture2D>	 WhiteTexture;

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1; 
		
		struct CameraData
		{
			mat4 ViewProjection;
		};
		CameraData CameraBuffer;
		Ref<UniformBuffer> CameraUniformBuffer;

		//Quad
		Ref<VertexArray>	   QuadVertexArray;
		Ref<VertexBuffer>	  QuadVertexBuffer;
		
		uint32_t QuadIndexCount = 0;
		
		Vertex* QuadVBBase = nullptr;
		Vertex* QuadVBPtr  = nullptr;

		//Tri
		Ref<VertexArray>   TriangleVertexArray;
		Ref<VertexBuffer> TriangleVertexBuffer;

		uint32_t TriIndexCount = 0;
		
		Vertex* TriVBBase = nullptr;
		Vertex* TriVBPtr  = nullptr; 

		//Circle
		Ref<VertexArray>	CircleVertexArray;
		Ref<VertexBuffer>  CircleVertexBuffer;

		uint32_t CircleIndexCount = 0;

		CircleVertex* CircleVBBase = nullptr;
		CircleVertex* CircleVBPtr  = nullptr;

		//Circle
		Ref<VertexArray>	LineVertexArray;
		Ref<VertexBuffer>  LineVertexBuffer;

		uint32_t LineVertexCount = 0;

		LineVertex* LineVBBase = nullptr;
		LineVertex* LineVBPtr  = nullptr;
	};
	static Renderer2DData s_Data;
	
	struct Vertex {
		vec3 Position;
		vec4 Colour;
		vec2 TexCoord;
		float TexIndex;
		float TilingFactor;

		//Editor Only
		int EntityID;
	};
	struct CircleVertex {
		vec3 Position;
		vec3 LocalPosition;
		vec4 Colour;
		float Thickness;
		float Fade;

		//Editor Only
		int EntityID;
	};
	struct LineVertex {
		vec3 Position;
		vec4 Colour;
		
		//Editor Only
		int EntityID;
	};

	static void SetupShape(const uint32_t Type, BufferLayout layout, uint32_t maxVertices, uint32_t maxIndices, uint32_t indicesPerShape, uint32_t verticesPerShape) {
		Ref<VertexArray> vArray  = VertexArray::Create();
		Ref<VertexBuffer> vBuffer = VertexBuffer::Create(maxVertices * sizeof(Vertex));
		vBuffer->SetLayout(layout);
		vArray->AddVertexBuffer(vBuffer); 
		
		uint32_t* indices = new uint32_t[maxIndices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < maxIndices; i += 6) {
			indices[i + 0] = offset + 0;
			indices[i + 1] = offset + 1;
			
			if (indicesPerShape >= 3)
				indices[i + 2] = offset + 2;

			if (indicesPerShape == 6) {
				indices[i + 3] = offset + 2;
				indices[i + 4] = offset + 3;
				indices[i + 5] = offset + 0;
			};
			
			offset += verticesPerShape;
		}

		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(indices, maxIndices);
		vArray->SetIndexBuffer(indexBuffer);

		if (Type == NB_QUAD) {
			s_Data.QuadVertexArray = vArray;
			s_Data.QuadVertexBuffer = vBuffer;
			s_Data.QuadVBBase = new Vertex[s_Data.MaxVertices];
		} else if (Type == NB_TRI) {
			s_Data.TriangleVertexArray = vArray;
			s_Data.TriangleVertexBuffer = vBuffer;
			s_Data.TriVBBase = new Vertex[s_Data.MaxVertices];
		} else if (Type == NB_CIRCLE) {
			s_Data.CircleVertexArray = vArray;
			s_Data.CircleVertexBuffer = vBuffer;
			s_Data.CircleVBBase = new CircleVertex[s_Data.MaxVertices];
		} else if (Type == NB_LINE) { 
			s_Data.LineVertexArray = vArray;
			s_Data.LineVertexBuffer = vBuffer;
			s_Data.LineVBBase = new LineVertex[s_Data.MaxVertices];
		}

		delete[] indices;
	}

	static void ResetBatch() {
		s_Data.QuadIndexCount = 0;
		s_Data.QuadVBPtr = s_Data.QuadVBBase;

		s_Data.TriIndexCount = 0;
		s_Data.TriVBPtr = s_Data.TriVBBase;

		s_Data.CircleIndexCount = 0;
		s_Data.CircleVBPtr = s_Data.CircleVBBase;

		s_Data.LineVertexCount = 0;
		s_Data.LineVBPtr = s_Data.LineVBBase;

		s_Data.TextureSlotIndex = 1;
	}
	
	void Renderer2D::Init() {
		NB_PROFILE_FUNCTION();

		BufferLayout layout = {
			{ShaderDataType::Float3, "position"},
			{ShaderDataType::Float4, "colour"},
			{ShaderDataType::Float2, "texCoord"},
			{ShaderDataType::Float, "texIndex"},
			{ShaderDataType::Float, "tilingFactor"},
			{ShaderDataType::Int, "entityID"}
		};

		BufferLayout CircleLayout = {
			{ShaderDataType::Float3, "position"},
			{ShaderDataType::Float3, "localPosition"},
			{ShaderDataType::Float4, "colour"},
			{ShaderDataType::Float, "thickness"},
			{ShaderDataType::Float, "fade"},
			{ShaderDataType::Int, "entityID"}
		};

		BufferLayout LineLayout = {
			{ShaderDataType::Float3, "position"},
			{ShaderDataType::Float4, "colour"},
			{ShaderDataType::Int, "entityID"}
		};

		SetupShape(NB_QUAD, layout, s_Data.MaxVertices, s_Data.MaxIndices, 6, 4);
		SetupShape(NB_CIRCLE, CircleLayout, s_Data.MaxVertices, s_Data.MaxIndices, 6, 4);
		SetupShape(NB_TRI, layout, s_Data.MaxVertices, s_Data.MaxIndices, 3, 3);
		SetupShape(NB_LINE, LineLayout, s_Data.MaxVertices, s_Data.MaxIndices, 2, 2);

		//White Texture
		s_Data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
		s_Data.TextureSlots[0] = s_Data.WhiteTexture;

		//Shaders
		s_Data.TextureShader = Shader::Create("assets/shaders/Default.glsl");
		s_Data.CircleShader = Shader::Create("assets/shaders/Circle.glsl");
		s_Data.LineShader = Shader::Create("assets/shaders/Line.glsl");
		
		int32_t samplers[s_Data.MaxTextureSlots];
		for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
			samplers[i] = i;

		s_Data.TextureShader->Bind();
		s_Data.TextureShader->SetIntArray("u_Textures", samplers, s_Data.MaxTextureSlots);
		
		//Camera Uniform
		s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer2DData::CameraData), 0);
	}

	void Renderer2D::Shutdown() {
		NB_PROFILE_FUNCTION();

		delete[] s_Data.QuadVBBase;
		delete[] s_Data.TriVBBase;
	}
	
	void Renderer2D::BeginScene(const Camera& camera, const mat4& transform) {
		NB_PROFILE_FUNCTION();
		
		s_Data.CameraBuffer.ViewProjection = camera.GetProjection() * inverse(transform);
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer2DData::CameraData));

		ResetBatch();
	}

	void Renderer2D::BeginScene(const EditorCamera& camera) {
		NB_PROFILE_FUNCTION();

		s_Data.CameraBuffer.ViewProjection = camera.GetViewProjection();
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer2DData::CameraData));

		ResetBatch();
	}
	
	void Renderer2D::Draw(const uint32_t type, const vec4* vertexPos, 
		const mat4& transform, const vec4& colour, Ref<Texture2D> texture, float tiling, uint32_t entityID) {
		NB_PROFILE_FUNCTION();
		if (texture == nullptr)
			texture = s_Data.WhiteTexture;

		if (type == NB_QUAD) {
			if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
				FlushAndReset();

			vec2* texCoords = new vec2[type];
			for (uint32_t i = 0; i < type; i+=4) {
				texCoords[i + 0] = { 0, 0 };
				texCoords[i + 1] = { 1, 0 };
				texCoords[i + 2] = { 1, 1 };
				texCoords[i + 3] = { 0, 1 };
			};

			s_Data.QuadVBPtr = CalculateVertexData(s_Data.QuadVBPtr, type, vertexPos, transform, colour, texture, texCoords, tiling, entityID);
			s_Data.QuadIndexCount += uint32_t(type * 1.5);
		}
		else if (type == NB_TRI) {
			if (s_Data.TriIndexCount >= s_Data.MaxIndices)
				FlushAndReset();

			vec2* texCoords = new vec2[type];
			for (uint32_t i = 0; i < type; i+=3) {
				texCoords[i + 0] = { 0.0f, 0.0f };
				texCoords[i + 1] = { 1.0f, 0.0f };
				texCoords[i + 2] = { 0.5f, 0.5f };
			};

			s_Data.TriVBPtr = CalculateVertexData(s_Data.TriVBPtr, type, vertexPos, transform, colour, texture, texCoords, tiling, entityID);
			s_Data.TriIndexCount += type;
		}
	}

	void Renderer2D::Draw(const uint32_t type, const vec4* vertexPos, const mat4& transform, const vec4& colour,
		const float thickness, const float fade, uint32_t entityID) {
		if (s_Data.CircleIndexCount >= s_Data.MaxIndices)
			FlushAndReset();

		vec2* texCoords = new vec2[4];
		for (uint32_t i = 0; i < 4; i += 4) {
			texCoords[i + 0] = { 0, 0 };
			texCoords[i + 1] = { 1, 0 };
			texCoords[i + 2] = { 1, 1 };
			texCoords[i + 3] = { 0, 1 };
		};

		s_Data.CircleVBPtr = CalculateVertexData(s_Data.CircleVBPtr, 4, vertexPos, transform, colour, thickness, fade, entityID);
		s_Data.CircleIndexCount += 6;
	}

	void Renderer2D::Draw(const uint32_t type, Entity& entity) {
		NB_PROFILE_FUNCTION();

		if (type == NB_RECT) {
			mat4 transform = entity.GetComponent<TransformComponent>().CalculateMatrix();
			vec4 colour = { 1, 1, 1, 1 };

			if (entity.HasComponent<SpriteRendererComponent>())
				colour = entity.GetComponent<SpriteRendererComponent>().Colour;
			else if (entity.HasComponent<CircleRendererComponent>())
				colour = entity.GetComponent<CircleRendererComponent>().Colour;

			vec3 p0 = transform * vec4(-0.5f, -0.5f, 0.0f, 1.0f);
			vec3 p1 = transform * vec4( 0.5f, -0.5f, 0.0f, 1.0f);
			vec3 p2 = transform * vec4( 0.5f,  0.5f, 0.0f, 1.0f);
			vec3 p3 = transform * vec4(-0.5f,  0.5f, 0.0f, 1.0f);
			
			DrawLine(p0, p1, colour, entity);
			DrawLine(p1, p2, colour, entity);
			DrawLine(p2, p3, colour, entity);
			DrawLine(p3, p0, colour, entity);

			return;
		}

		uint32_t size = type;
		if (type == NB_CIRCLE)
			size = 4;

		vec4* vertexPos = new vec4[size];

		if (type == NB_QUAD || type == NB_CIRCLE) {
			vertexPos[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
			vertexPos[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
			vertexPos[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
			vertexPos[3] = { -0.5f,  0.5f, 0.0f, 1.0f };
		} else if (type == NB_TRI) {
			vertexPos[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
			vertexPos[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
			vertexPos[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
		} else if (type == NB_LINE) {
			vertexPos[0] = { -0.5f, 0.0f, 0.0f, 1.0f };
			vertexPos[1] = {  0.5f, 0.0f, 0.0f, 1.0f };
		}

		if (type == NB_CIRCLE) {
			mat4 transform = entity.GetComponent<TransformComponent>().CalculateMatrix();
			auto& circleRenderer = entity.GetComponent<CircleRendererComponent>();

			Draw(type, vertexPos, transform, circleRenderer.Colour, circleRenderer.Thickness, circleRenderer.Fade, entity);
		}

		else if (type == NB_LINE) {
			NB_WARN("Render2D::Draw(type = NB_LINE) is depreciated, Use Render2D::DrawLine(const vec3&, const vec3&, const vec4&, int) instead");
			mat4 transform = entity.GetComponent<TransformComponent>().CalculateMatrix();
			//TODO: Line Renderer Component
			
			DrawLine(vertexPos[0] * transform[3], vertexPos[1] * transform[3], { 1.0f, 1.0f, 1.0f, 1.0f });
		} else {
			mat4 transform = entity.GetComponent<TransformComponent>().CalculateMatrix();
			auto& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();

			Draw(type, vertexPos, transform, spriteRenderer.Colour, spriteRenderer.Texture, spriteRenderer.Tiling, entity);
		}
	}

	void Renderer2D::Draw(const uint32_t type, const mat4& transform, const vec4& colour, Ref<Texture2D> texture, float tiling) {
		switch (type)
		{
			case NB_RECT: {
				vec3 p0 = transform * vec4(-0.5f, -0.5f, 0.0f, 1.0f);
				vec3 p1 = transform * vec4(0.5f, -0.5f, 0.0f, 1.0f);
				vec3 p2 = transform * vec4(0.5f, 0.5f, 0.0f, 1.0f);
				vec3 p3 = transform * vec4(-0.5f, 0.5f, 0.0f, 1.0f);

				DrawLine(p0, p1, colour);
				DrawLine(p1, p2, colour);
				DrawLine(p2, p3, colour);
				DrawLine(p3, p0, colour);

				return;
			}

			case NB_LINE: {
				NB_WARN("Render2D::Draw(type = NB_LINE) is depreciated, please use Render2D::DrawLine(const vec3&, const vec3&, const vec4&, int)");

				vec4 vertexPos[] = { { -0.5f, 0.0f, 0.0f, 1.0f }, { 0.5f, 0.0f, 0.0f, 1.0f } };
				DrawLine(vertexPos[0] * transform[3], vertexPos[1] * transform[3], colour);

				return;
			}
			
			case NB_CIRCLE: {
				DrawCircle(transform, colour);
				return;
			}

		}

		vec4* vertexPos = new vec4[type];

		if (type == NB_QUAD) {
			vertexPos[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
			vertexPos[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
			vertexPos[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
			vertexPos[3] = { -0.5f,  0.5f, 0.0f, 1.0f };
		}
		else if (type == NB_TRI) {
			vertexPos[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
			vertexPos[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
			vertexPos[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
		}

		Draw(type, vertexPos, transform, colour, texture, tiling);
	}

	void Renderer2D::DrawCircle(const mat4& transform, const vec4& colour, const float thickness, const float fade, int EntityID) {
		vec4 vertexPos[4] = {
			{ -0.5f, -0.5f, 0.0f, 1.0f },
			{  0.5f, -0.5f, 0.0f, 1.0f },
			{  0.5f,  0.5f, 0.0f, 1.0f },
			{ -0.5f,  0.5f, 0.0f, 1.0f },
		};

		Draw(NB_CIRCLE, vertexPos, transform, colour, thickness, fade, EntityID);
	}

	void Renderer2D::DrawLine(const vec3& p0, const vec3& p1, const vec4& colour, int entityID) {
		s_Data.LineVBPtr->Position = p0;
		s_Data.LineVBPtr->Colour = colour;
		s_Data.LineVBPtr->EntityID = entityID;
		s_Data.LineVBPtr++;

		s_Data.LineVBPtr->Position = p1;
		s_Data.LineVBPtr->Colour = colour;
		s_Data.LineVBPtr->EntityID = entityID;
		s_Data.LineVBPtr++;

		s_Data.LineVertexCount += 2;
	}

	Vertex* Renderer2D::CalculateVertexData(Vertex* vertexPtr, const uint32_t vertexCount, const vec4* vertexPos, 
		const mat4& transform, const vec4& colour, Ref<Texture2D> texture, vec2* texCoord, float tiling, uint32_t entityID) {
		NB_PROFILE_FUNCTION();

		float textureIndex = 0.0f;

		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++) {
			if (*s_Data.TextureSlots[i].get() == *texture.get()) {
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f) {
			if (s_Data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
				FlushAndReset();

			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
			s_Data.TextureSlotIndex++;
		}
		
		for (size_t i = 0; i < vertexCount; i++) {
			vertexPtr->Position = transform * vertexPos[i];
			vertexPtr->Colour = colour;
			vertexPtr->TexCoord = texCoord[i];
			vertexPtr->TexIndex = textureIndex;
			vertexPtr->TilingFactor = tiling;
			vertexPtr->EntityID = entityID;
			vertexPtr++;
		}

		return vertexPtr;
	}

	CircleVertex* Renderer2D::CalculateVertexData(CircleVertex* vertexPtr, const uint32_t vertexCount, const vec4* vertexPos,
		const mat4& transform, const vec4& colour, float thickness, float fade, uint32_t entityID) {
		NB_PROFILE_FUNCTION();

		for (size_t i = 0; i < vertexCount; i++) {
			vertexPtr->Position = transform * vertexPos[i];
			vertexPtr->LocalPosition = vertexPos[i] * 2.0f;
			vertexPtr->Colour = colour;
			vertexPtr->Thickness = thickness;
			vertexPtr->Fade = fade;
			vertexPtr->EntityID = entityID;
			vertexPtr++;
		}

		return vertexPtr;
	}

	void Renderer2D::EndScene() {
		NB_PROFILE_FUNCTION();
		
		s_Data.TextureShader->Bind();
		
		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
			s_Data.TextureSlots[i]->Bind(i);
		
		if (s_Data.TriIndexCount) {
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.TriVBPtr - (uint8_t*)s_Data.TriVBBase);
			s_Data.TriangleVertexBuffer->SetData(s_Data.TriVBBase, dataSize);
			
			RenderCommand::DrawIndexed(s_Data.TriangleVertexArray, s_Data.TriIndexCount);
		}

		if (s_Data.QuadIndexCount) {
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVBPtr - (uint8_t*)s_Data.QuadVBBase);
			s_Data.QuadVertexBuffer->SetData(s_Data.QuadVBBase, dataSize);

			RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
		}
		
		if (s_Data.CircleIndexCount) {
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.CircleVBPtr - (uint8_t*)s_Data.CircleVBBase);
			s_Data.CircleVertexBuffer->SetData(s_Data.CircleVBBase, dataSize);
			
			s_Data.CircleShader->Bind();
			RenderCommand::DrawIndexed(s_Data.CircleVertexArray, s_Data.CircleIndexCount);
		}

		if (s_Data.LineVertexCount) {
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.LineVBPtr - (uint8_t*)s_Data.LineVBBase);
			s_Data.LineVertexBuffer->SetData(s_Data.LineVBBase, dataSize);

			s_Data.LineShader->Bind();
			RenderCommand::DrawLines(s_Data.LineVertexArray, s_Data.LineVertexCount);
		}
	}

	void Renderer2D::FlushAndReset() {
		EndScene();
		ResetBatch();
	}
}