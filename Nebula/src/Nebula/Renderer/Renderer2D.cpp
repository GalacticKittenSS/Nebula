#include "nbpch.h"
#include "Renderer2D.h"

#include "Buffer.h"
#include "Vertex_Array.h"
#include "Shader.h"
#include "Render_Command.h"
#include "UniformBuffer.h"

#include "Nebula/AssetManager/AssetManager.h"
#include "Nebula/Scene/Components.h"

#include "MSDFData.h"

namespace Nebula {
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec4 Colour;
		glm::vec2 TexCoord;
		float TexIndex;
		float TilingFactor;

		//Editor Only
		int EntityID;
	};

	struct CircleVertex
	{
		glm::vec3 Position;
		glm::vec3 LocalPosition;
		glm::vec4 Colour;
		float Thickness;
		float Fade;

		//Editor Only
		int EntityID;
	};

	struct LineVertex
	{
		glm::vec3 Position;
		glm::vec4 Colour;

		//Editor Only
		int EntityID;
	};

	struct TextVertex
	{
		glm::vec3 Position;
		glm::vec4 Colour;
		glm::vec2 TexCoord;

		//Editor Only
		int EntityID;
	};

	struct Renderer2DData 
	{
		static const uint32_t MaxSprites = 10000;
		static const uint32_t MaxTextureSlots = 32;
		static const uint32_t MaxVertices  = MaxSprites * 4;
		static const uint32_t MaxIndices   = MaxSprites * 6;
		
		Ref<Shader>		TextureShader;
		Ref<Shader>		 CircleShader;
		Ref<Shader>		   LineShader;
		Ref<Shader>		   TextShader;
		Ref<Texture2D>	 WhiteTexture;

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1; // 0 = White Texture 

		struct CameraData
		{
			glm::mat4 ViewProjection;
		};
		CameraData CameraBuffer;
		Ref<UniformBuffer> CameraUniformBuffer;

		//Quad
		Ref<VertexArray>	   QuadVertexArray;
		Ref<VertexBuffer>	  QuadVertexBuffer;
		
		glm::vec4* QuadVertexPos = new glm::vec4[4];
		glm::vec2* QuadTexCoords = new glm::vec2[4];
		uint32_t QuadIndexCount = 0;
		
		Vertex* QuadVBBase = nullptr;
		Vertex* QuadVBPtr  = nullptr;

		//Tri
		Ref<VertexArray>   TriangleVertexArray;
		Ref<VertexBuffer> TriangleVertexBuffer;

		glm::vec4* TriVertexPos = new glm::vec4[3];
		glm::vec2* TriTexCoords = new glm::vec2[3];
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

		glm::vec4* LineVertexPos = new glm::vec4[2];
		uint32_t LineVertexCount = 0;

		LineVertex* LineVBBase = nullptr;
		LineVertex* LineVBPtr  = nullptr;

		Ref<Texture2D> FontTexture;
		
		// Text
		Ref<VertexArray>	TextVertexArray;
		Ref<VertexBuffer>	TextVertexBuffer;
		Ref<Texture2D>		FontAtlasTexture;

		uint32_t TextIndexCount = 0;

		TextVertex* TextVBBase = nullptr;
		TextVertex* TextVBPtr = nullptr;
	};
	static Renderer2DData s_Data;
	
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
		else if (Type == NB_STRING) {
			s_Data.TextVertexArray = vArray;
			s_Data.TextVertexBuffer = vBuffer;
			s_Data.TextVBBase = new TextVertex[s_Data.MaxVertices];
		}
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

		s_Data.TextIndexCount = 0;
		s_Data.TextVBPtr = s_Data.TextVBBase;

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

		BufferLayout Textlayout = {
			{ShaderDataType::Float3, "position"},
			{ShaderDataType::Float4, "colour"},
			{ShaderDataType::Float2, "texCoord"},
			{ShaderDataType::Int, "entityID"}
		};

		SetupShape(NB_QUAD, layout, s_Data.MaxVertices, s_Data.MaxIndices, 6, 4);
		SetupShape(NB_TRI, layout, s_Data.MaxVertices, s_Data.MaxIndices, 3, 3);
		SetupShape(NB_CIRCLE, CircleLayout, s_Data.MaxVertices, s_Data.MaxIndices, 6, 4);
		SetupShape(NB_LINE, LineLayout, s_Data.MaxVertices, s_Data.MaxIndices, 2, 2);
		SetupShape(NB_STRING, Textlayout, s_Data.MaxVertices, s_Data.MaxIndices, 6, 4);

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
		s_Data.WhiteTexture = Texture2D::Create(TextureSpecification());
		uint32_t whiteTextureData = 0xffffffff;
		s_Data.WhiteTexture->SetData(Buffer(&whiteTextureData, sizeof(uint32_t)));
		s_Data.TextureSlots[0] = s_Data.WhiteTexture;

		//Shaders
		s_Data.TextureShader = Shader::Create("Resources/shaders/Default.glsl");
		//s_Data.LineShader = Shader::Create("Resources/shaders/Line.glsl");
		s_Data.CircleShader = Shader::Create("Resources/shaders/Circle.glsl");
		s_Data.TextShader = Shader::Create("Resources/shaders/Text.glsl");
		
		// OpenGL
		{
			int32_t samplers[s_Data.MaxTextureSlots];
			for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
				samplers[i] = i;

			s_Data.TextureShader->Bind();
			s_Data.TextureShader->SetIntArray("u_Textures", samplers, s_Data.MaxTextureSlots);
		}
		
		// Vulkan (Fill Texture Array with Default Texture)
		s_Data.TextureShader->SetTextureArray("u_Textures", s_Data.WhiteTexture);

		//Camera Uniform
		s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer2DData::CameraData), 0);
		s_Data.TextureShader->SetUniformBuffer("u_ViewProjection", s_Data.CameraUniformBuffer);
		s_Data.CircleShader->SetUniformBuffer("u_ViewProjection", s_Data.CameraUniformBuffer);
		s_Data.TextShader->SetUniformBuffer("u_ViewProjection", s_Data.CameraUniformBuffer);
	}

	void Renderer2D::Shutdown() {
		NB_PROFILE_FUNCTION();

		delete[] s_Data.QuadVBBase;
		delete[] s_Data.TriVBBase;
		delete[] s_Data.CircleVBBase;
		delete[] s_Data.LineVBBase;
		delete[] s_Data.TextVBBase;

		delete[] s_Data.QuadVertexPos;
		delete[] s_Data.TriVertexPos;
		delete[] s_Data.LineVertexPos;

		delete[] s_Data.QuadTexCoords;
		delete[] s_Data.TriTexCoords;
	}
	
	void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform) {
		NB_PROFILE_FUNCTION();
		
		s_Data.CameraBuffer.ViewProjection = camera.GetProjection() * glm::inverse(transform);
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer2DData::CameraData));

		ResetBatch();
	}

	void Renderer2D::BeginScene(const EditorCamera& camera) {
		NB_PROFILE_FUNCTION();

		s_Data.CameraBuffer.ViewProjection = camera.GetViewProjection();
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer2DData::CameraData));

		ResetBatch();
	}

	void Renderer2D::DrawString(const std::string& text, Ref<Font> font,
		const glm::mat4& transform, const TextParams& params, uint32_t entityID)
	{
		NB_PROFILE_FUNCTION();
		if (text.empty() || !font)
			return;

		if (s_Data.TextIndexCount >= s_Data.MaxIndices)
			FlushAndReset();

		if (s_Data.FontAtlasTexture != font->GetAtlasTexture())
		{
			FlushAndReset();
			s_Data.FontAtlasTexture = font->GetAtlasTexture();
		}

		const auto& fontGeometry = font->GetMSDFData()->FontGeometry;
		const auto& metrics = fontGeometry.getMetrics();
		
		Ref<Texture2D> fontAtlas = font->GetAtlasTexture();
		NB_ASSERT(fontAtlas);
		
		float texelWidth = 1.0f / fontAtlas->GetWidth();
		float texelHeight = 1.0f / fontAtlas->GetHeight();

		double x = 0.0f, y = 0.0f;
		double fsScale = 1.0f / (metrics.ascenderY - metrics.descenderY);

		const double spaceGlyphAdvance = fontGeometry.getGlyph(' ')->getAdvance();
		
		for (uint32_t i = 0; i < text.length(); i++)
		{
			char character = text[i];
			if (character == '\r')
				continue;

			if (character == '\n')
			{
				x = 0;
				y -= fsScale * metrics.lineHeight + params.LineSpacing;
				continue;
			}

			if (character == ' ')
			{
				double advance = spaceGlyphAdvance;
				if (i < text.size() - 1)
				{
					char nextChar = text[i + 1];
					double dAdvance;
					fontGeometry.getAdvance(dAdvance, character, nextChar);
					advance = dAdvance;
				}

				x += fsScale * advance;
				continue;
			}

			if (character == '\t')
			{
				x += 4.0f * (fsScale * spaceGlyphAdvance);
				continue;
			}

			auto glyph = fontGeometry.getGlyph(character);
			
			if (!glyph)
				glyph = fontGeometry.getGlyph('?');

			if (!glyph)
				continue;

			double al, ab, ar, at;
			glyph->getQuadAtlasBounds(al, ab, ar, at);
			
			double pl, pb, pr, pt;
			glyph->getQuadPlaneBounds(pl, pb, pr, pt);

			double x0 = x + pl * fsScale;
			double y0 = y + pb * fsScale;
			double x1 = x + pr * fsScale;
			double y1 = y + pt * fsScale;

			double u0 = al * texelWidth;
			double v0 = ab * texelHeight;
			double u1 = ar * texelWidth;
			double v1 = at * texelHeight;

			s_Data.TextVBPtr->Position = transform * glm::vec4(x0, y0, 0.0f, 1.0f);
			s_Data.TextVBPtr->TexCoord = glm::vec2(u0, v0);
			s_Data.TextVBPtr->Colour = params.Colour;
			s_Data.TextVBPtr->EntityID = entityID;
			s_Data.TextVBPtr++;

			s_Data.TextVBPtr->Position = transform * glm::vec4(x1, y0, 0.0f, 1.0f);
			s_Data.TextVBPtr->TexCoord = glm::vec2(u1, v0);
			s_Data.TextVBPtr->Colour = params.Colour;
			s_Data.TextVBPtr->EntityID = entityID;
			s_Data.TextVBPtr++;

			s_Data.TextVBPtr->Position = transform * glm::vec4(x1, y1, 0.0f, 1.0f);
			s_Data.TextVBPtr->TexCoord = glm::vec2(u1, v1);
			s_Data.TextVBPtr->Colour = params.Colour;
			s_Data.TextVBPtr->EntityID = entityID;
			s_Data.TextVBPtr++;

			s_Data.TextVBPtr->Position = transform * glm::vec4(x0, y1, 0.0f, 1.0f);
			s_Data.TextVBPtr->TexCoord = glm::vec2(u0, v1);
			s_Data.TextVBPtr->Colour = params.Colour;
			s_Data.TextVBPtr->EntityID = entityID;
			s_Data.TextVBPtr++;

			s_Data.TextIndexCount += 6;

			if (i < text.length() - 1)
			{
				double advance = glyph->getAdvance();
				fontGeometry.getAdvance(advance, text[i], text[i + 1]);
				x += fsScale * advance + params.Kerning;
			}
		}
	}

	void Renderer2D::DrawTri(const uint32_t vertexCount, const glm::vec4* vertexPos, glm::vec2* texCoords,
		const glm::mat4& transform, const Material& mat, uint32_t entityID)
	{
		NB_PROFILE_FUNCTION();

		if (s_Data.TriIndexCount >= s_Data.MaxIndices)
			FlushAndReset();

		float textureIndex = GetTextureIndex(mat.Texture ? mat.Texture : s_Data.WhiteTexture);

		for (size_t i = 0; i < vertexCount; i++) 
		{
			s_Data.TriVBPtr->Position = transform * vertexPos[i];
			s_Data.TriVBPtr->Colour = mat.Colour;
			s_Data.TriVBPtr->TexCoord = texCoords[i];
			s_Data.TriVBPtr->TexIndex = textureIndex;
			s_Data.TriVBPtr->TilingFactor = mat.Tiling;
			s_Data.TriVBPtr->EntityID = entityID;
			s_Data.TriVBPtr++;
		}

		s_Data.TriIndexCount += vertexCount;
	}

	void Renderer2D::DrawQuad(const uint32_t vertexCount, const glm::vec4* vertexPos, glm::vec2* texCoords,
		const glm::mat4& transform, const Material& mat, uint32_t entityID)
	{
		NB_PROFILE_FUNCTION();
		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
			FlushAndReset();

		float textureIndex = GetTextureIndex(mat.Texture ? mat.Texture : s_Data.WhiteTexture);

		for (size_t i = 0; i < vertexCount; i++)
		{
			s_Data.QuadVBPtr->Position = transform * vertexPos[i];
			s_Data.QuadVBPtr->Colour = mat.Colour;
			s_Data.QuadVBPtr->TexCoord = texCoords[i];
			s_Data.QuadVBPtr->TexIndex = textureIndex;
			s_Data.QuadVBPtr->TilingFactor = mat.Tiling;
			s_Data.QuadVBPtr->EntityID = entityID;
			s_Data.QuadVBPtr++;
		}

		s_Data.QuadIndexCount += uint32_t(vertexCount * 1.5);
	}

	void Renderer2D::DrawCircle(const glm::mat4& transform, const Material& mat, const float thickness, const float fade, uint32_t entityID)
	{
		NB_PROFILE_FUNCTION();

		if (s_Data.CircleIndexCount >= s_Data.MaxIndices)
			FlushAndReset();

		for (size_t i = 0; i < 4; i++)
		{
			s_Data.CircleVBPtr->Position = transform * s_Data.QuadVertexPos[i];
			s_Data.CircleVBPtr->LocalPosition = s_Data.QuadVertexPos[i] * 2.0f;
			s_Data.CircleVBPtr->Colour = mat.Colour;
			s_Data.CircleVBPtr->Thickness = thickness;
			s_Data.CircleVBPtr->Fade = fade;
			s_Data.CircleVBPtr->EntityID = entityID;
			s_Data.CircleVBPtr++;
		}

		s_Data.CircleIndexCount += 6;
	}

	void Renderer2D::DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& colour, int entityID) 
	{
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

	void Renderer2D::DrawRect(const glm::mat4& transform, const Material& material, int entityID)
	{
		glm::vec3 p0 = transform * s_Data.QuadVertexPos[0];
		glm::vec3 p1 = transform * s_Data.QuadVertexPos[1];
		glm::vec3 p2 = transform * s_Data.QuadVertexPos[2];
		glm::vec3 p3 = transform * s_Data.QuadVertexPos[3];

		DrawLine(p0, p1, material.Colour);
		DrawLine(p1, p2, material.Colour);
		DrawLine(p2, p3, material.Colour);
		DrawLine(p3, p0, material.Colour);
	}

	void Renderer2D::Draw(const SpriteRendererComponent& sprite, const glm::mat4& transform, const Material& material, int entityID)
	{
		if (material.Texture)
		{
			Ref<SubTexture2D> SubT = SubTexture2D::CreateFromCoords(material.Texture, sprite.SubTextureOffset, sprite.SubTextureCellSize, sprite.SubTextureCellNum);
			DrawQuad(4, s_Data.QuadVertexPos, SubT->GetTextureCoords(), transform, material, entityID);
		}
		else
		{
			DrawQuad(4, s_Data.QuadVertexPos, s_Data.QuadTexCoords, transform, material, entityID);
		}
		
	}
	
	void Renderer2D::Draw(const CircleRendererComponent& circle, const glm::mat4& transform, const Material& material, int entityID)
	{
		DrawCircle(transform, material, circle.Thickness, circle.Fade, entityID);
	}
	
	void Renderer2D::Draw(const StringRendererComponent& string, const glm::mat4& transform, int entityID)
	{
		TextParams params = { string.Colour, string.Kerning, string.LineSpacing };
		DrawString(string.Text, string.GetFont(), transform, params, entityID);
	}

	float Renderer2D::GetTextureIndex(const Ref<Texture2D>& texture) 
	{
		float textureIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++) 
		{
			if (*s_Data.TextureSlots[i].get() == *texture.get()) 
			{
				return (float)i;
			}
		}

		if (s_Data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
			FlushAndReset();

		textureIndex = (float)s_Data.TextureSlotIndex;
		s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
		s_Data.TextureSlotIndex++;
		return textureIndex;
	}

	void Renderer2D::EndScene() {
		NB_PROFILE_FUNCTION();
		
		if (s_Data.QuadIndexCount || s_Data.TriIndexCount) {
			s_Data.TextureShader->Bind();
		
			for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
				s_Data.TextureSlots[i]->Bind(i);
		}

		RenderCommand::BeginRecording();

		if (s_Data.TriIndexCount) {
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.TriVBPtr - (uint8_t*)s_Data.TriVBBase);
			s_Data.TriangleVertexBuffer->SetData(s_Data.TriVBBase, dataSize);
			
			s_Data.TextureShader->Bind();
			RenderCommand::DrawIndexed(s_Data.TriangleVertexArray, s_Data.TriIndexCount);
		}

		if (s_Data.QuadIndexCount) {
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVBPtr - (uint8_t*)s_Data.QuadVBBase);
			s_Data.QuadVertexBuffer->SetData(s_Data.QuadVBBase, dataSize);

			s_Data.TextureShader->Bind();
			RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
		}
		
		if (s_Data.CircleIndexCount) {
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.CircleVBPtr - (uint8_t*)s_Data.CircleVBBase);
			s_Data.CircleVertexBuffer->SetData(s_Data.CircleVBBase, dataSize);
			
			s_Data.CircleShader->Bind();
			RenderCommand::DrawIndexed(s_Data.CircleVertexArray, s_Data.CircleIndexCount);
		}

		if (s_Data.LineVertexCount && false) {
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.LineVBPtr - (uint8_t*)s_Data.LineVBBase);
			s_Data.LineVertexBuffer->SetData(s_Data.LineVBBase, dataSize);

			s_Data.LineShader->Bind();
			RenderCommand::DrawLines(s_Data.LineVertexArray, s_Data.LineVertexCount);
		}

		if (s_Data.TextIndexCount) {
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.TextVBPtr - (uint8_t*)s_Data.TextVBBase);
			s_Data.TextVertexBuffer->SetData(s_Data.TextVBBase, dataSize);

			s_Data.TextShader->Bind();
			s_Data.FontAtlasTexture->Bind();

			RenderCommand::DrawIndexed(s_Data.TextVertexArray, s_Data.TextIndexCount);
		}

		RenderCommand::EndRecording();
	}

	void Renderer2D::FlushAndReset() {
		EndScene();
		ResetBatch();
	}
}
