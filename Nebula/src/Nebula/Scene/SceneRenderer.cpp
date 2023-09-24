#include "nbpch.h"
#include "SceneRenderer.h"

#include "Nebula/Core/Application.h"
#include "Nebula/AssetManager/TextureImporter.h"

#include "Nebula/Scene/Scene.h"
#include "Nebula/Renderer/MSDFData.h"
#include "Nebula/Renderer/Render_Command.h"

#include "Platform/Vulkan/VulkanAPI.h"

namespace Nebula
{
	struct Defaults
	{
		glm::vec2* TextureCoords = new glm::vec2[4];
		glm::vec2* CubeTexCoords = new glm::vec2[24];
		glm::vec4* QuadVertexPos = new glm::vec4[4];
		glm::vec4* CubeVertexPos = new glm::vec4[24];
		glm::vec4* LineVertexPos = new glm::vec4[2];

		Ref<Texture2D> WhiteTexture;
		Ref<Texture2D> SkyTexture;
	};
	static Defaults s_Defaults;

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

	struct VertexData
	{
		std::array<Ref<Texture2D>, 32> TextureSlots;
		uint32_t TextureSlotIndex = 1; // 0 = White Texture 

		//Sky
		Ref<VertexArray>	   SkyVertexArray;
		Ref<VertexBuffer>	  SkyVertexBuffer;

		//Quad
		Ref<VertexArray>	   QuadVertexArray;
		Ref<VertexBuffer>	  QuadVertexBuffer;

		uint32_t QuadIndexCount = 0;

		Vertex* QuadVBBase = nullptr;
		Vertex* QuadVBPtr = nullptr;

		//Circle
		Ref<VertexArray>	CircleVertexArray;
		Ref<VertexBuffer>  CircleVertexBuffer;

		uint32_t CircleIndexCount = 0;

		CircleVertex* CircleVBBase = nullptr;
		CircleVertex* CircleVBPtr = nullptr;

		//Line
		Ref<VertexArray>	LineVertexArray;
		Ref<VertexBuffer>  LineVertexBuffer;

		uint32_t LineVertexCount = 0;

		LineVertex* LineVBBase = nullptr;
		LineVertex* LineVBPtr = nullptr;

		// Text
		Ref<VertexArray>	TextVertexArray;
		Ref<VertexBuffer>	TextVertexBuffer;
		Ref<Texture2D>		FontAtlasTexture;

		uint32_t TextIndexCount = 0;

		TextVertex* TextVBBase = nullptr;
		TextVertex* TextVBPtr = nullptr;
	};
	static VertexData s_Data;

	static void SetupBuffers(Ref<VertexArray>& vertexArray, Ref<VertexBuffer>& vertexBuffer, uint32_t indicesPerShape, uint32_t verticesPerShape)
	{
		vertexArray = VertexArray::Create();
		vertexBuffer = VertexBuffer::Create(SceneRenderer::Settings::MaxVertices * sizeof(Vertex));
		vertexArray->AddVertexBuffer(vertexBuffer);

		uint32_t maxIndices = SceneRenderer::Settings::MaxSprites * indicesPerShape;
		uint32_t* indices = new uint32_t[maxIndices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < maxIndices; i += indicesPerShape) {
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
		vertexArray->SetIndexBuffer(indexBuffer);
		delete[] indices;
	}

	static void ResetBatch() {
		s_Data.QuadIndexCount = 0;
		s_Data.QuadVBPtr = s_Data.QuadVBBase;

		s_Data.CircleIndexCount = 0;
		s_Data.CircleVBPtr = s_Data.CircleVBBase;

		s_Data.LineVertexCount = 0;
		s_Data.LineVBPtr = s_Data.LineVBBase;

		s_Data.TextIndexCount = 0;
		s_Data.TextVBPtr = s_Data.TextVBBase;

		s_Data.TextureSlotIndex = 1;
	}
	
	void SceneRenderer::Setup()
	{
		s_Defaults.QuadVertexPos[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		s_Defaults.QuadVertexPos[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
		s_Defaults.QuadVertexPos[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
		s_Defaults.QuadVertexPos[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

		s_Defaults.TextureCoords[0] = { 0.0f, 0.0f };
		s_Defaults.TextureCoords[1] = { 1.0f, 0.0f };
		s_Defaults.TextureCoords[2] = { 1.0f, 1.0f };
		s_Defaults.TextureCoords[3] = { 0.0f, 1.0f };
		
		s_Defaults.LineVertexPos[0] = { -0.5f, 0.0f, 0.0f, 1.0f };
		s_Defaults.LineVertexPos[1] = { 0.5f, 0.0f, 0.0f, 1.0f };
		
		{
			//Front
			s_Defaults.CubeVertexPos[0] = { -0.5f, -0.5f, -0.5f, 1 };
			s_Defaults.CubeVertexPos[1] = { 0.5f, -0.5f, -0.5f, 1 };
			s_Defaults.CubeVertexPos[2] = { 0.5f,  0.5f, -0.5f, 1 };
			s_Defaults.CubeVertexPos[3] = { -0.5f,  0.5f, -0.5f, 1 };

			//Left
			s_Defaults.CubeVertexPos[4] = { 0.5f, -0.5f, -0.5f, 1 };
			s_Defaults.CubeVertexPos[5] = { 0.5f, -0.5f,  0.5f, 1 };
			s_Defaults.CubeVertexPos[6] = { 0.5f,  0.5f,  0.5f, 1 };
			s_Defaults.CubeVertexPos[7] = { 0.5f,  0.5f, -0.5f, 1 };

			//Top
			s_Defaults.CubeVertexPos[8] = { -0.5f,  0.5f, -0.5f, 1 };
			s_Defaults.CubeVertexPos[9] = { 0.5f,  0.5f, -0.5f, 1 };
			s_Defaults.CubeVertexPos[10] = { 0.5f,  0.5f,  0.5f, 1 };
			s_Defaults.CubeVertexPos[11] = { -0.5f,  0.5f,  0.5f, 1 };

			//Right
			s_Defaults.CubeVertexPos[12] = { -0.5f, -0.5f,  0.5f, 1 };
			s_Defaults.CubeVertexPos[13] = { -0.5f, -0.5f, -0.5f, 1 };
			s_Defaults.CubeVertexPos[14] = { -0.5f,  0.5f, -0.5f, 1 };
			s_Defaults.CubeVertexPos[15] = { -0.5f,  0.5f,  0.5f, 1 };

			//Back
			s_Defaults.CubeVertexPos[16] = { -0.5f, -0.5f,  0.5f, 1 };
			s_Defaults.CubeVertexPos[17] = { 0.5f, -0.5f,  0.5f, 1 };
			s_Defaults.CubeVertexPos[18] = { 0.5f,  0.5f,  0.5f, 1 };
			s_Defaults.CubeVertexPos[19] = { -0.5f,  0.5f,  0.5f, 1 };

			//Bottom
			s_Defaults.CubeVertexPos[20] = { -0.5f, -0.5f, -0.5f, 1 };
			s_Defaults.CubeVertexPos[21] = { 0.5f, -0.5f, -0.5f, 1 };
			s_Defaults.CubeVertexPos[22] = { 0.5f, -0.5f,  0.5f, 1 };
			s_Defaults.CubeVertexPos[23] = { -0.5f, -0.5f,  0.5f, 1 };

		}
		
		{
			//Front
			s_Defaults.CubeTexCoords[0] = { 0.50f, 0.345f };
			s_Defaults.CubeTexCoords[1] = { 0.25f, 0.345f };
			s_Defaults.CubeTexCoords[2] = { 0.25f, 0.66f };
			s_Defaults.CubeTexCoords[3] = { 0.50f, 0.66f };

			//Left
			s_Defaults.CubeTexCoords[4] = { 0.25f, 0.345f };
			s_Defaults.CubeTexCoords[5] = { 0.00f, 0.345f };
			s_Defaults.CubeTexCoords[6] = { 0.00f, 0.66f };
			s_Defaults.CubeTexCoords[7] = { 0.25f, 0.66f };

			//Top
			s_Defaults.CubeTexCoords[8] = { 0.49f, 0.66f };
			s_Defaults.CubeTexCoords[9] = { 0.26f, 0.66f };
			s_Defaults.CubeTexCoords[10] = { 0.26f, 1.00f };
			s_Defaults.CubeTexCoords[11] = { 0.49f, 1.00f };

			//Right
			s_Defaults.CubeTexCoords[12] = { 0.75f, 0.345f };
			s_Defaults.CubeTexCoords[13] = { 0.50f, 0.345f };
			s_Defaults.CubeTexCoords[14] = { 0.50f, 0.66f };
			s_Defaults.CubeTexCoords[15] = { 0.75f, 0.66f };

			//Back
			s_Defaults.CubeTexCoords[16] = { 0.75f, 0.345f };
			s_Defaults.CubeTexCoords[17] = { 1.00f, 0.345f };
			s_Defaults.CubeTexCoords[18] = { 1.00f, 0.66f };
			s_Defaults.CubeTexCoords[19] = { 0.75f, 0.66f };

			//Bottom
			s_Defaults.CubeTexCoords[20] = { 0.499f, 0.345f };
			s_Defaults.CubeTexCoords[21] = { 0.256f, 0.345f };
			s_Defaults.CubeTexCoords[22] = { 0.256f, 0.00f };
			s_Defaults.CubeTexCoords[23] = { 0.499f, 0.00f };
		}
		
		{
			TextureSpecification spec;
			spec.ImGuiUsable = false;
			s_Defaults.WhiteTexture = Texture2D::Create(spec);
			
			int data = 0xFFFFFFFF;
			s_Defaults.WhiteTexture->SetData({ &data, sizeof(int) });
		}

		s_Defaults.SkyTexture = TextureImporter::CreateTexture2D("Resources/Textures/bg.png", false);
		
		s_Data.QuadVBBase	= new Vertex[Settings::MaxVertices];
		s_Data.CircleVBBase = new CircleVertex[Settings::MaxVertices];
		s_Data.LineVBBase	= new LineVertex[Settings::MaxVertices];
		s_Data.TextVBBase	= new TextVertex[Settings::MaxVertices];
		s_Data.TextureSlots[0] = s_Defaults.WhiteTexture;
	}

	void SceneRenderer::Shutdown()
	{
		delete[] s_Data.QuadVBBase;
		delete[] s_Data.CircleVBBase;
		delete[] s_Data.LineVBBase;
		delete[] s_Data.TextVBBase;

		delete[] s_Defaults.QuadVertexPos;
		delete[] s_Defaults.LineVertexPos;
		delete[] s_Defaults.TextureCoords;
	}

	SceneRenderer::SceneRenderer()
	{
		Window& window = Application::Get().GetWindow();

		// Create Render Passes
		{
			RenderPassSpecification spec;
			spec.ClearOnLoad = true;
			spec.SingleWrite = true;
			spec.Attachments = {
				{ ImageFormat::RGBA8, ImageLayout::Undefined, ImageLayout::ColourAttachment }, 
				ImageFormat::RED_INT, ImageFormat::DEPTH24STENCIL8 
			};
			m_Data.SkyPass = RenderPass::Create(spec);
		
			spec.Attachments[0] = { ImageFormat::RGBA8, ImageLayout::ColourAttachment, ImageLayout::ColourAttachment };
			spec.ClearOnLoad = false;
			m_Data.GeometryPass = RenderPass::Create(spec);

			spec.Attachments[0] = { ImageFormat::RGBA8, ImageLayout::ColourAttachment, ImageLayout::ShaderReadOnly };
			m_Data.ColliderPass = RenderPass::Create(spec);
		}

		// Create Framebuffer
		{
			FrameBufferSpecification spec;
			spec.Attachments = { ImageFormat::RGBA8, ImageFormat::RED_INT, ImageFormat::DEPTH24STENCIL8 };
			spec.Width = window.GetWidth();
			spec.Height = window.GetHeight();
			spec.SwapChainTarget = false;
			spec.RenderPass = m_Data.GeometryPass;
			spec.ClearColour = { 0.1f, 0.1f, 0.1f, 1.0f };
			spec.DepthClearValue = 0.0f;
			m_Data.Frambuffer = FrameBuffer::Create(spec);
		}
		
		PipelineSpecification pipelineSpec;
		pipelineSpec.RenderPass = m_Data.GeometryPass;
		pipelineSpec.Shape = PipelineShape::Triangles;

		//Camera Uniform
		m_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(RenderData::CameraData), 0);

		// Quad Setup
		SetupBuffers(s_Data.QuadVertexArray, s_Data.QuadVertexBuffer, 6, 4);
		SetupBuffers(s_Data.SkyVertexArray, s_Data.SkyVertexBuffer, 6, 4);
		
		// Circle Setup
		{
			SetupBuffers(s_Data.CircleVertexArray, s_Data.CircleVertexBuffer, 6, 4);
			
			m_Data.CircleShader = Shader::Create("Resources/shaders/Circle.glsl");
			m_Data.CircleShader->SetUniformBuffer("u_ViewProjection", m_Data.CameraUniformBuffer);

			pipelineSpec.Shader = m_Data.CircleShader;
			m_Data.CirclePipeline = Pipeline::Create(pipelineSpec);
		}

		// Line Setup
		{
			SetupBuffers(s_Data.LineVertexArray, s_Data.LineVertexBuffer, 2, 2);
			
			m_Data.LineShader = Shader::Create("Resources/shaders/Line.glsl");
			m_Data.LineShader->SetUniformBuffer("u_ViewProjection", m_Data.CameraUniformBuffer);

			pipelineSpec.Shader = m_Data.LineShader;
			pipelineSpec.Shape = PipelineShape::Lines;
			m_Data.LinePipeline = Pipeline::Create(pipelineSpec);
		}

		// Text Setup
		{
			SetupBuffers(s_Data.TextVertexArray, s_Data.TextVertexBuffer, 6, 4);
			
			m_Data.TextShader = Shader::Create("Resources/shaders/Text.glsl");
			m_Data.TextShader->SetUniformBuffer("u_ViewProjection", m_Data.CameraUniformBuffer);

			pipelineSpec.Shader = m_Data.TextShader;
			pipelineSpec.Shape = PipelineShape::Triangles;
			m_Data.TextPipeline = Pipeline::Create(pipelineSpec);
		}

		// Texture Shader
		m_Data.TextureShader = Shader::Create("Resources/shaders/Default.glsl");
		m_Data.TextureShader->SetUniformBuffer("u_ViewProjection", m_Data.CameraUniformBuffer);

		pipelineSpec.Shader = m_Data.TextureShader;
		m_Data.TexturePipeline = Pipeline::Create(pipelineSpec);
		m_Data.TextureShader->SetTextureArray("u_Textures", s_Defaults.WhiteTexture);
	}

	void SceneRenderer::Resize(uint32_t width, uint32_t height)
	{
		m_Data.Frambuffer->Resize(width, height);
	}

	glm::vec2 SceneRenderer::GetFramebufferSize()
	{
		FrameBufferSpecification& spec = m_Data.Frambuffer->GetFrameBufferSpecifications();
		return { spec.Width, spec.Height };
	}

	Ref<Image2D> SceneRenderer::GetFinalImage()
	{
		return m_Data.Frambuffer->GetColourAttachmentImage(0); 
	}

	int SceneRenderer::ReadImage(uint32_t x, uint32_t y)
	{
		return m_Data.Frambuffer->ReadPixel(1, x, y);
	}

	void SceneRenderer::RenderSprite(const glm::mat4& transform, Ref<Material> mat, const SpriteRendererComponent& sprite, int entityID)
	{
		if (s_Data.QuadIndexCount >= Settings::MaxIndices)
			FlushAndReset();

		float textureIndex = mat->Texture ? GetTextureIndex(mat->Texture) : 0.0f;

		for (size_t i = 0; i < 4; i++)
		{
			s_Data.QuadVBPtr->Position = transform * s_Defaults.QuadVertexPos[i];
			s_Data.QuadVBPtr->Colour = mat->Colour;
			s_Data.QuadVBPtr->TexCoord = s_Defaults.TextureCoords[i];
			s_Data.QuadVBPtr->TexIndex = textureIndex;
			s_Data.QuadVBPtr->TilingFactor = mat->Tiling;
			s_Data.QuadVBPtr->EntityID = entityID;
			s_Data.QuadVBPtr++;
		}

		s_Data.QuadIndexCount += 6;
	}

	void SceneRenderer::RenderCircle(const glm::mat4& transform, Ref<Material> mat, const CircleRendererComponent& circle, int entityID)
	{
		if (s_Data.CircleIndexCount >= Settings::MaxIndices)
			FlushAndReset();

		for (size_t i = 0; i < 4; i++)
		{
			s_Data.CircleVBPtr->Position = transform * s_Defaults.QuadVertexPos[i];
			s_Data.CircleVBPtr->LocalPosition = s_Defaults.QuadVertexPos[i] * 2.0f;
			s_Data.CircleVBPtr->Colour = mat->Colour;
			s_Data.CircleVBPtr->Thickness = circle.Thickness;
			s_Data.CircleVBPtr->Fade = circle.Fade;
			s_Data.CircleVBPtr->EntityID = entityID;
			s_Data.CircleVBPtr++;
		}

		s_Data.CircleIndexCount += 6;
	}

	void SceneRenderer::RenderString(const glm::mat4& transform, Ref<Font> font, const StringRendererComponent& string, int entityID)
	{
		if (s_Data.TextIndexCount >= Settings::MaxIndices)
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

		for (uint32_t i = 0; i < string.Text.length(); i++)
		{
			char character = string.Text[i];
			if (character == '\r')
				continue;

			if (character == '\n')
			{
				x = 0;
				y -= fsScale * metrics.lineHeight + string.LineSpacing;
				continue;
			}

			if (character == ' ')
			{
				double advance = spaceGlyphAdvance;
				if (i < string.Text.size() - 1)
				{
					char nextChar = string.Text[i + 1];
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
			s_Data.TextVBPtr->Colour = string.Colour;
			s_Data.TextVBPtr->EntityID = entityID;
			s_Data.TextVBPtr++;

			s_Data.TextVBPtr->Position = transform * glm::vec4(x1, y0, 0.0f, 1.0f);
			s_Data.TextVBPtr->TexCoord = glm::vec2(u1, v0);
			s_Data.TextVBPtr->Colour = string.Colour;
			s_Data.TextVBPtr->EntityID = entityID;
			s_Data.TextVBPtr++;

			s_Data.TextVBPtr->Position = transform * glm::vec4(x1, y1, 0.0f, 1.0f);
			s_Data.TextVBPtr->TexCoord = glm::vec2(u1, v1);
			s_Data.TextVBPtr->Colour = string.Colour;
			s_Data.TextVBPtr->EntityID = entityID;
			s_Data.TextVBPtr++;

			s_Data.TextVBPtr->Position = transform * glm::vec4(x0, y1, 0.0f, 1.0f);
			s_Data.TextVBPtr->TexCoord = glm::vec2(u0, v1);
			s_Data.TextVBPtr->Colour = string.Colour;
			s_Data.TextVBPtr->EntityID = entityID;
			s_Data.TextVBPtr++;

			s_Data.TextIndexCount += 6;

			if (i < string.Text.length() - 1)
			{
				double advance = glyph->getAdvance();
				fontGeometry.getAdvance(advance, string.Text[i], string.Text[i + 1]);
				x += fsScale * advance + string.Kerning;
			}
		}
	}

	void SceneRenderer::RenderCircleCollider(glm::mat4& transform, const CircleColliderComponent& circleCollider, const glm::vec3& projectionCollider, int entityID)
	{
		glm::vec3 wTranslation, wRotation, wScale;
		Maths::DecomposeTransform(transform, wTranslation, wRotation, wScale);

		glm::vec3 scale = wScale.x * glm::vec3(circleCollider.Radius * 2.0f);
		glm::mat4 new_transform = glm::translate(wTranslation) * glm::toMat4(glm::quat(wRotation))
			* glm::translate(glm::vec3(circleCollider.Offset, -projectionCollider.z)) * glm::scale(scale);

		if (s_Data.CircleIndexCount >= Settings::MaxIndices)
			FlushAndReset();

		for (size_t i = 0; i < 4; i++)
		{
			s_Data.CircleVBPtr->Position = new_transform * s_Defaults.QuadVertexPos[i];
			s_Data.CircleVBPtr->LocalPosition = s_Defaults.QuadVertexPos[i] * 2.0f;
			s_Data.CircleVBPtr->Colour = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
			s_Data.CircleVBPtr->Thickness = 0.05f;
			s_Data.CircleVBPtr->Fade = 0.005f;
			s_Data.CircleVBPtr->EntityID = entityID;
			s_Data.CircleVBPtr++;
		}

		s_Data.CircleIndexCount += 6;
	}

	void DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& colour, int entityID)
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

	void SceneRenderer::RenderBoxCollider(glm::mat4& transform, const BoxCollider2DComponent& boxCollider, float zIndex, int entityID)
	{
		glm::vec3 wTranslation, wRotation, wScale;
		Maths::DecomposeTransform(transform, wTranslation, wRotation, wScale);

		glm::vec3 scale = wScale * glm::vec3(boxCollider.Size, 0.0f) * 2.0f;
		glm::mat4 new_transform = glm::translate(wTranslation) * glm::toMat4(glm::quat(wRotation)) *
			glm::translate(glm::vec3(boxCollider.Offset, zIndex)) * glm::scale(scale);
		
		glm::vec3 p0 = new_transform * s_Defaults.QuadVertexPos[0];
		glm::vec3 p1 = new_transform * s_Defaults.QuadVertexPos[1];
		glm::vec3 p2 = new_transform * s_Defaults.QuadVertexPos[2];
		glm::vec3 p3 = new_transform * s_Defaults.QuadVertexPos[3];

		DrawLine(p0, p1, { 0.0f, 1.0f, 0.0f, 1.0f }, entityID);
		DrawLine(p1, p2, { 0.0f, 1.0f, 0.0f, 1.0f }, entityID);
		DrawLine(p2, p3, { 0.0f, 1.0f, 0.0f, 1.0f }, entityID);
		DrawLine(p3, p0, { 0.0f, 1.0f, 0.0f, 1.0f }, entityID);
	}

	void SceneRenderer::SkyPrePass(glm::vec3 position)
	{
		float textureIndex = GetTextureIndex(s_Defaults.SkyTexture);
		glm::mat4 transform = glm::translate(position) * glm::scale(glm::vec3(1000.0f));

		for (size_t i = 0; i < 24; i++)
		{
			s_Data.QuadVBPtr->Position = transform * s_Defaults.CubeVertexPos[i];
			s_Data.QuadVBPtr->Colour = glm::vec4(1.0f);
			s_Data.QuadVBPtr->TexCoord = s_Defaults.CubeTexCoords[i];
			s_Data.QuadVBPtr->TexIndex = textureIndex;
			s_Data.QuadVBPtr->TilingFactor = 1.0f;
			s_Data.QuadVBPtr->EntityID = -1;
			s_Data.QuadVBPtr++;
		}

		s_Data.QuadIndexCount += 36;
	}
	
	void SceneRenderer::SkyPass()
	{
		m_Data.SkyPass->Bind();

		m_Data.TextureShader->ResetDescriptorSet(1);
		m_Data.TextureShader->SetTextureArray("u_Textures", s_Defaults.WhiteTexture);

		m_Data.TextureShader->Bind();
		m_Data.TexturePipeline->Bind();

		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
			s_Data.TextureSlots[i]->Bind(i);

		m_Data.TexturePipeline->BindDescriptorSet();

		uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVBPtr - (uint8_t*)s_Data.QuadVBBase);
		s_Data.SkyVertexBuffer->SetData(s_Data.QuadVBBase, dataSize);
		RenderCommand::DrawIndexed(s_Data.SkyVertexArray, s_Data.QuadIndexCount);
		
		m_Data.SkyPass->Unbind();
	}

	void SceneRenderer::GeometryPrePass()
	{
		auto spriteGroup = m_Context->m_Registry.group<WorldTransformComponent, MaterialComponent>(entt::get<SpriteRendererComponent>);
		for (auto id : spriteGroup)
		{
			auto [transform, material, sprite] = spriteGroup.get<WorldTransformComponent, MaterialComponent, SpriteRendererComponent>(id);
			Ref<Material> mat = AssetManager::GetAsset<Material>(material.Material);
			RenderSprite(transform.Transform, mat, sprite, (int)id);
		}

		auto circleGroup = m_Context->m_Registry.view<WorldTransformComponent, MaterialComponent, CircleRendererComponent>();
		for (auto id : circleGroup)
		{
			auto [transform, material, circle] = circleGroup.get<WorldTransformComponent, MaterialComponent, CircleRendererComponent>(id);
			Ref<Material> mat = AssetManager::GetAsset<Material>(material.Material);
			RenderCircle(transform.Transform, mat, circle, (int)id);
		}

		auto stringGroup = m_Context->m_Registry.view<WorldTransformComponent, StringRendererComponent>();
		for (auto id : stringGroup)
		{
			auto [transform, string] = stringGroup.get<WorldTransformComponent, StringRendererComponent>(id);
			Ref<Font> font = string.GetFont();
			RenderString(transform.Transform, font, string, (int)id);
		}
	}

	void SceneRenderer::GeometryPass()
	{
		m_Data.GeometryPass->Bind();

		if (s_Data.QuadIndexCount) {
			m_Data.TextureShader->ResetDescriptorSet(1);
			m_Data.TextureShader->SetTextureArray("u_Textures", s_Defaults.WhiteTexture);

			m_Data.TextureShader->Bind();
			m_Data.TexturePipeline->Bind();

			for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
				s_Data.TextureSlots[i]->Bind(i);

			m_Data.TexturePipeline->BindDescriptorSet();

			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVBPtr - (uint8_t*)s_Data.QuadVBBase);
			s_Data.QuadVertexBuffer->SetData(s_Data.QuadVBBase, dataSize);
			RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
		}

		if (s_Data.CircleIndexCount) {
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.CircleVBPtr - (uint8_t*)s_Data.CircleVBBase);
			s_Data.CircleVertexBuffer->SetData(s_Data.CircleVBBase, dataSize);

			m_Data.CircleShader->Bind();
			m_Data.CirclePipeline->Bind();
			m_Data.CirclePipeline->BindDescriptorSet();

			RenderCommand::DrawIndexed(s_Data.CircleVertexArray, s_Data.CircleIndexCount);
		}

		if (s_Data.LineVertexCount) {
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.LineVBPtr - (uint8_t*)s_Data.LineVBBase);
			s_Data.LineVertexBuffer->SetData(s_Data.LineVBBase, dataSize);

			m_Data.LineShader->Bind();
			m_Data.LinePipeline->Bind();
			m_Data.LinePipeline->BindDescriptorSet();

			RenderCommand::DrawLines(s_Data.LineVertexArray, s_Data.LineVertexCount);
		}

		if (s_Data.TextIndexCount) {
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.TextVBPtr - (uint8_t*)s_Data.TextVBBase);
			s_Data.TextVertexBuffer->SetData(s_Data.TextVBBase, dataSize);

			m_Data.TextShader->Bind();
			m_Data.TextPipeline->Bind();
			s_Data.FontAtlasTexture->Bind();
			m_Data.TextPipeline->BindDescriptorSet();

			RenderCommand::DrawIndexed(s_Data.TextVertexArray, s_Data.TextIndexCount);
		}

		m_Data.GeometryPass->Unbind();
	}

	void SceneRenderer::ColliderPrePass(glm::vec3 forward)
	{
		// Calculate z index for translation
		float zIndex = 0.001f;
		glm::vec3 projectionCollider = forward * glm::vec3(zIndex);

		auto circleGroup = m_Context->m_Registry.view<WorldTransformComponent, CircleColliderComponent>();
		for (auto id : circleGroup)
		{
			auto [wtc, cc] = circleGroup.get<WorldTransformComponent, CircleColliderComponent>(id);
			RenderCircleCollider(wtc.Transform, cc, projectionCollider, (int)id);
		}

		auto boxGroup = m_Context->m_Registry.view<WorldTransformComponent, BoxCollider2DComponent>();
		for (auto id : boxGroup)
		{
			auto [wtc, bc2d] = boxGroup.get<WorldTransformComponent, BoxCollider2DComponent>(id);
			RenderBoxCollider(wtc.Transform, bc2d, zIndex, (int)id);
		}
	}

	void SceneRenderer::ColliderPass()
	{
		m_Data.ColliderPass->Bind();

		if (s_Data.CircleIndexCount) {
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.CircleVBPtr - (uint8_t*)s_Data.CircleVBBase);
			s_Data.CircleVertexBuffer->SetData(s_Data.CircleVBBase, dataSize);

			m_Data.CircleShader->Bind();
			m_Data.CirclePipeline->Bind();
			m_Data.CirclePipeline->BindDescriptorSet();

			RenderCommand::DrawIndexed(s_Data.CircleVertexArray, s_Data.CircleIndexCount);
		}

		if (s_Data.LineVertexCount) {
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.LineVBPtr - (uint8_t*)s_Data.LineVBBase);
			s_Data.LineVertexBuffer->SetData(s_Data.LineVBBase, dataSize);

			m_Data.LineShader->Bind();
			m_Data.LinePipeline->Bind();
			m_Data.LinePipeline->BindDescriptorSet();

			RenderCommand::DrawLines(s_Data.LineVertexArray, s_Data.LineVertexCount);
		}

		m_Data.ColliderPass->Unbind();
	}

	void SceneRenderer::Render(const EditorCamera& camera)
	{
		NB_ASSERT(m_Context);

		m_Data.CameraBuffer.ViewProjection = camera.GetViewProjection();
		m_Data.CameraUniformBuffer->SetData(&m_Data.CameraBuffer, sizeof(RenderData::CameraData));
		
		ResetBatch();
		
		m_Data.Frambuffer->Bind();
		RenderCommand::BeginRecording();
		m_Data.Frambuffer->ClearDepthAttachment(0);

		if (m_Settings.ShowSky)
		{
			SkyPrePass(camera.GetPosition());
			SkyPass();
		}

		GeometryPrePass();
		GeometryPass();
		
		if (m_Settings.ShowColliders)
		{
			ColliderPrePass(camera.GetForwardDirection());
			ColliderPass();
		}
		else
		{
			Ref<Image2D> image = m_Data.Frambuffer->GetColourAttachmentImage(0);
			image->TransitionImageLayout(ImageLayout::ColourAttachment, ImageLayout::ShaderReadOnly);
		}

		RenderCommand::EndRecording();
		m_Data.Frambuffer->Unbind();
	}

	void SceneRenderer::Render(const Camera& camera, const glm::mat4& transform)
	{
		NB_ASSERT(m_Context);

		m_Data.CameraBuffer.ViewProjection = camera.GetProjection() * glm::inverse(transform);
		m_Data.CameraUniformBuffer->SetData(&m_Data.CameraBuffer, sizeof(RenderData::CameraData));

		ResetBatch();

		m_Data.Frambuffer->Bind();
		RenderCommand::BeginRecording();
		m_Data.Frambuffer->ClearDepthAttachment(0);

		if (m_Settings.ShowSky)
		{
			SkyPrePass(transform[3]);
			SkyPass();
		}
		
		GeometryPrePass();
		GeometryPass();

		if (m_Settings.ShowColliders)
		{
			ColliderPrePass({ 0.0f, 0.0f, 1.0f });
			ColliderPass();
		}
		else
		{
			Ref<Image2D> image = m_Data.Frambuffer->GetColourAttachmentImage(0);
			image->TransitionImageLayout(ImageLayout::ColourAttachment, ImageLayout::ShaderReadOnly);
		}

		RenderCommand::EndRecording();
		m_Data.Frambuffer->Unbind();
	}

	void SceneRenderer::FlushAndReset() 
	{
		GeometryPass();
		ResetBatch();

		RenderCommand::EndRecording();
		RenderCommand::BeginRecording();
	}

	float SceneRenderer::GetTextureIndex(const Ref<Texture2D>& texture)
	{
		float textureIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i].get() == *texture.get())
			{
				return (float)i;
			}
		}

		if (s_Data.TextureSlotIndex >= Settings::MaxTextureSlots)
			FlushAndReset();

		textureIndex = (float)s_Data.TextureSlotIndex;
		s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
		s_Data.TextureSlotIndex++;
		return textureIndex;
	}
}