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
		
		vec4* QuadVertexPos = new vec4[4];
		vec2* QuadTexCoords = new vec2[4];
		uint32_t QuadIndexCount = 0;
		
		Vertex* QuadVBBase = nullptr;
		Vertex* QuadVBPtr  = nullptr;

		//Tri
		Ref<VertexArray>   TriangleVertexArray;
		Ref<VertexBuffer> TriangleVertexBuffer;

		vec4* TriVertexPos = new vec4[3];
		vec2* TriTexCoords = new vec2[3];
		uint32_t TriIndexCount = 0;
		
		Vertex* TriVBBase = nullptr;
		Vertex* TriVBPtr  = nullptr; 

		//Circle
		Ref<VertexArray>	CircleVertexArray;
		Ref<VertexBuffer>  CircleVertexBuffer;

		uint32_t CircleIndexCount = 0;

		CircleVertex* CircleVBBase = nullptr;
		CircleVertex* CircleVBPtr  = nullptr;

		//Line
		Ref<VertexArray>	LineVertexArray;
		Ref<VertexBuffer>  LineVertexBuffer;

		vec4* LineVertexPos = new vec4[2];
		uint32_t LineVertexCount = 0;

		LineVertex* LineVBBase = nullptr;
		LineVertex* LineVBPtr  = nullptr;

		Ref<Texture2D> FontTexture;
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

	static void SetupShape(const uint32_t Type, BufferLayout layout, uint32_t maxVertices, uint32_t maxIndices, 
		uint32_t indicesPerShape, uint32_t verticesPerShape) {
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
		delete[] indices;

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
	}

	static void ResetBatch() {
		NB_PROFILE_FUNCTION();

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

		s_Data.QuadVertexPos[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPos[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPos[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPos[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

		s_Data.QuadTexCoords[0] = { 0.0f, 0.0f };
		s_Data.QuadTexCoords[1] = { 1.0f, 0.0f };
		s_Data.QuadTexCoords[2] = { 1.0f, 1.0f };
		s_Data.QuadTexCoords[3] = { 0.0f, 1.0f };

		s_Data.TriVertexPos[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.TriVertexPos[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.TriVertexPos[2] = {  0.5f,  0.5f, 0.0f, 1.0f };

		s_Data.TriTexCoords[0] = { 0.0f, 0.0f };
		s_Data.TriTexCoords[1] = { 1.0f, 0.0f };
		s_Data.TriTexCoords[2] = { 0.5f, 0.5f };

		s_Data.LineVertexPos[0] = { -0.5f, 0.0f, 0.0f, 1.0f };
		s_Data.LineVertexPos[1] = {  0.5f, 0.0f, 0.0f, 1.0f };

		//White Texture
		s_Data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
		s_Data.TextureSlots[0] = s_Data.WhiteTexture;

		//Shaders
		s_Data.TextureShader = Shader::Create("Resources/shaders/Default.glsl");
		s_Data.CircleShader = Shader::Create("Resources/shaders/Circle.glsl");
		s_Data.LineShader = Shader::Create("Resources/shaders/Line.glsl");
		
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
		delete[] s_Data.CircleVBBase;
		delete[] s_Data.LineVBBase;

		delete[] s_Data.QuadVertexPos;
		delete[] s_Data.TriVertexPos;
		delete[] s_Data.LineVertexPos;

		delete[] s_Data.QuadTexCoords;
		delete[] s_Data.TriTexCoords;
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

	void Renderer2D::SetBackCulling(bool cull) {
		s_Data.LineShader->SetBackfaceCulling(cull);
		s_Data.CircleShader->SetBackfaceCulling(cull);
		s_Data.TextureShader->SetBackfaceCulling(cull);
	}

	void Renderer2D::DrawString(const std::string& text, Font* font, 
		const mat4& transform, const vec4& colour, uint32_t entityID) 
	{
		NB_PROFILE_FUNCTION();
		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
			FlushAndReset();

		float x = 0.0f;
		const vec2& fontScale = font->GetScale();
		float textureIndex = GetTextureIndex(font->GetTexture());

		for (uint32_t i = 0; i < text.length(); i++) {
			FontGlyph glyph = font->GetGlyph(&text[i]);
			if (!glyph) continue;

			if (i > 0)
				x += glyph.GetKerning(&text[i - 1]) / fontScale.x;

			float x0 = x + glyph.offset_x() / fontScale.x;
			float y0 =	   glyph.offset_y() / fontScale.y;
			float x1 = x0 + glyph.width()  / fontScale.x;
			float y1 = y0 - glyph.height() / fontScale.y;
			float z = i / 1000.0f;

			float u0 = glyph.s0();
			float v0 = glyph.t0();
			float u1 = glyph.s1();
			float v1 = glyph.t1();

			s_Data.QuadVBPtr->Position = transform * vec4(x0, y0, z, 1.0f);
			s_Data.QuadVBPtr->TexCoord = vec2(u0, v0);
			s_Data.QuadVBPtr->TexIndex = textureIndex;
			s_Data.QuadVBPtr->TilingFactor = 1.0f;
			s_Data.QuadVBPtr->Colour = colour;
			s_Data.QuadVBPtr->EntityID = entityID;
			s_Data.QuadVBPtr++;

			s_Data.QuadVBPtr->Position = transform * vec4(x0, y1, z, 1.0f);
			s_Data.QuadVBPtr->TexCoord = vec2(u0, v1);
			s_Data.QuadVBPtr->TexIndex = textureIndex;
			s_Data.QuadVBPtr->TilingFactor = 1.0f;
			s_Data.QuadVBPtr->Colour = colour;
			s_Data.QuadVBPtr->EntityID = entityID;
			s_Data.QuadVBPtr++;

			s_Data.QuadVBPtr->Position = transform * vec4(x1, y1, z, 1.0f);
			s_Data.QuadVBPtr->TexCoord = vec2(u1, v1);
			s_Data.QuadVBPtr->TexIndex = textureIndex;
			s_Data.QuadVBPtr->TilingFactor = 1.0f;
			s_Data.QuadVBPtr->Colour = colour;
			s_Data.QuadVBPtr->EntityID = entityID;
			s_Data.QuadVBPtr++;

			s_Data.QuadVBPtr->Position = transform * vec4(x1, y0, z, 1.0f);
			s_Data.QuadVBPtr->TexCoord = vec2(u1, v0);
			s_Data.QuadVBPtr->TexIndex = textureIndex;
			s_Data.QuadVBPtr->TilingFactor = 1.0f;
			s_Data.QuadVBPtr->Colour = colour;
			s_Data.QuadVBPtr->EntityID = entityID;
			s_Data.QuadVBPtr++;

			s_Data.QuadIndexCount += 6;
			x += glyph.advance_x() / fontScale.x;
		}
	}

	void Renderer2D::DrawTri(const uint32_t vertexCount, const vec4* vertexPos, vec2* texCoords,
		const mat4& transform, const vec4& colour, Ref<Texture2D> texture, float tiling, uint32_t entityID)
	{
		NB_PROFILE_FUNCTION();

		if (s_Data.TriIndexCount >= s_Data.MaxIndices)
			FlushAndReset();

		texture = texture ? texture : s_Data.WhiteTexture;
		s_Data.TriVBPtr = CalculateVertexData(s_Data.TriVBPtr, vertexCount, vertexPos, transform, colour, texture, texCoords, tiling, entityID);
		s_Data.TriIndexCount += vertexCount;
	}

	void Renderer2D::DrawQuad(const uint32_t vertexCount, const vec4* vertexPos, vec2* texCoords,
		const mat4& transform, const vec4& colour, Ref<Texture2D> texture, float tiling, uint32_t entityID)
	{
		NB_PROFILE_FUNCTION();
		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
			FlushAndReset();

		texture = texture ? texture : s_Data.WhiteTexture;
		s_Data.QuadVBPtr = CalculateVertexData(s_Data.QuadVBPtr, vertexCount, vertexPos, transform, colour, texture, texCoords, tiling, entityID);
		s_Data.QuadIndexCount += uint32_t(vertexCount * 1.5);
	}

	void Renderer2D::DrawCircle(const mat4& transform, const vec4& colour, const float thickness, const float fade, uint32_t entityID) {
		NB_PROFILE_FUNCTION();

		if (s_Data.CircleIndexCount >= s_Data.MaxIndices)
			FlushAndReset();

		s_Data.CircleVBPtr = CalculateVertexData(s_Data.CircleVBPtr, 4, s_Data.QuadVertexPos, transform, colour, thickness, fade, entityID);
		s_Data.CircleIndexCount += 6;
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

	void Renderer2D::Draw(const uint32_t type, Entity& entity) {
		NB_PROFILE_FUNCTION();

		mat4 transform = entity.GetComponent<WorldTransformComponent>().Transform;
		switch (type)
		{
		case NB_RECT: {
			vec4 colour = { 1, 1, 1, 1 };

			if (entity.HasComponent<SpriteRendererComponent>())
				colour = entity.GetComponent<SpriteRendererComponent>().Colour;
			else if (entity.HasComponent<CircleRendererComponent>())
				colour = entity.GetComponent<CircleRendererComponent>().Colour;

			vec3 p0 = transform * vec4(-0.5f, -0.5f, 0.0f, 1.0f);
			vec3 p1 = transform * vec4(0.5f, -0.5f, 0.0f, 1.0f);
			vec3 p2 = transform * vec4(0.5f, 0.5f, 0.0f, 1.0f);
			vec3 p3 = transform * vec4(-0.5f, 0.5f, 0.0f, 1.0f);

			DrawLine(p0, p1, colour, entity);
			DrawLine(p1, p2, colour, entity);
			DrawLine(p2, p3, colour, entity);
			DrawLine(p3, p0, colour, entity);

			break;
		}
		case NB_CIRCLE: {
			auto& circleRenderer = entity.GetComponent<CircleRendererComponent>();
			DrawCircle(transform, circleRenderer.Colour, circleRenderer.Thickness, circleRenderer.Fade, entity);
			break;
		}
		case NB_LINE: {
			//TODO: Line Renderer Component
			DrawLine(s_Data.LineVertexPos[0] * transform[3], s_Data.LineVertexPos[1] * transform[3], vec4(1.0f));
			break;
		}
		case NB_QUAD: {
			auto& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();

			if (spriteRenderer.Texture && spriteRenderer.Texture->IsLoaded()) {
				Ref<SubTexture2D> SubT = SubTexture2D::CreateFromCoords(spriteRenderer.Texture,
					spriteRenderer.SubTextureOffset, spriteRenderer.SubTextureCellSize, spriteRenderer.SubTextureCellNum);
				DrawQuad(4, s_Data.QuadVertexPos, SubT->GetTextureCoords(), transform,
					spriteRenderer.Colour, spriteRenderer.Texture, spriteRenderer.Tiling, entity);
			}
			else
				DrawQuad(4, s_Data.QuadVertexPos, s_Data.QuadTexCoords, transform, 
					spriteRenderer.Colour, s_Data.WhiteTexture, spriteRenderer.Tiling, entity);
			break;
		}
		case NB_STRING: {
			auto& stringRender = entity.GetComponent<StringRendererComponent>();
			const char* fontStrings[] = StringRenderFontTypeStrings;

			DrawString(stringRender.Text, stringRender.Ft, transform, stringRender.Colour, entity);
			break;
		}
		default:
			NB_ERROR("[Renderer2D] Unknown Type Specified");
			break;
		}
	}

	void Renderer2D::Draw(const uint32_t type, const mat4& transform, const vec4& colour, Ref<Texture2D> texture, float tiling) {
		switch (type) {
		case NB_RECT: {
			vec3 p0 = transform * s_Data.QuadVertexPos[0];
			vec3 p1 = transform * s_Data.QuadVertexPos[1];
			vec3 p2 = transform * s_Data.QuadVertexPos[2];
			vec3 p3 = transform * s_Data.QuadVertexPos[3];

			DrawLine(p0, p1, colour);
			DrawLine(p1, p2, colour);
			DrawLine(p2, p3, colour);
			DrawLine(p3, p0, colour);

			break;
		}

		case NB_LINE:
			DrawLine(s_Data.LineVertexPos[0] * transform[3], s_Data.LineVertexPos[1] * transform[3], colour);
			break;

		case NB_CIRCLE:
			DrawCircle(transform, colour);
			break;

		case NB_QUAD:
			DrawQuad(4, s_Data.QuadVertexPos, s_Data.QuadTexCoords, transform, colour, texture, tiling);
			break;

		case NB_TRI:
			DrawTri(3, s_Data.TriVertexPos, s_Data.TriTexCoords, transform, colour, texture, tiling);
			break;

		case NB_STRING:
			NB_WARN("[Render2D] Please Use Renderer2D::DrawString to render text");
			break;
		}
	}
	
	void Renderer2D::Draw(const uint32_t type, const vec4* vertexPos, vec2* texCoords, 
		const mat4& transform, const vec4& colour, Ref<Texture2D> texture, float tiling) {
		uint32_t size = sizeof(vertexPos) / sizeof(vec4);
		switch (type) {
			case NB_QUAD:
				if (!texCoords) {
					texCoords = new vec2[size];
					for (uint32_t i = 0; i < size; i += 4) {
						texCoords[i + 0] = s_Data.QuadVertexPos[0];
						texCoords[i + 1] = s_Data.QuadVertexPos[1];
						texCoords[i + 2] = s_Data.QuadVertexPos[2];
						texCoords[i + 3] = s_Data.QuadVertexPos[3];
					};
				}

				DrawQuad(size, vertexPos, texCoords, transform, colour, texture, tiling);
				break;

			case NB_TRI:
				if (!texCoords) {
					texCoords = new vec2[size];
					for (uint32_t i = 0; i < size; i += 4) {
						texCoords[i + 0] = s_Data.TriVertexPos[0];
						texCoords[i + 1] = s_Data.TriVertexPos[1];
						texCoords[i + 2] = s_Data.TriVertexPos[2];
					};
				}
				DrawTri(size, vertexPos, texCoords, transform, colour, texture, tiling);
				break;
		}
	}

	float Renderer2D::GetTextureIndex(const Ref<Texture2D>& texture) {
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
		return textureIndex;
	}

	Vertex* Renderer2D::CalculateVertexData(Vertex* vertexPtr, const uint32_t vertexCount, const vec4* vertexPos,
		const mat4& transform, const vec4& colour, Ref<Texture2D> texture, vec2* texCoord, float tiling, uint32_t entityID)
	{
		NB_PROFILE_FUNCTION();

		float textureIndex = GetTextureIndex(texture);
		
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

	CircleVertex* Renderer2D::CalculateVertexData(CircleVertex* vertexPtr, const uint32_t vertexCount, const vec4* vertexPos, const mat4& transform, const vec4& colour, float thickness, float fade, uint32_t entityID) 
	{
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
		
		if (s_Data.QuadIndexCount || s_Data.TriIndexCount) {
			s_Data.TextureShader->Bind();
		
			for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
				s_Data.TextureSlots[i]->Bind(i);
		}

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
