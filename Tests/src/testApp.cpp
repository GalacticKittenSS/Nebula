#include <Nebula.h>

using namespace Nebula;

class App : public Application {
public:
	App(): cam(-2.0f, 2.0f, -2.0f, 2.0f) { Start(); }

	~App() { }

	void Start() {
		BufferLayout layout = {
			{ShaderDataType::Float3, "position"},
			{ShaderDataType::Float4, "colour"}
		};

		float triangleVertexes[6 * 7] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.75f, 0.0f, 1.0f, //BOTTOM LEFT  (V1)
			 0.5f, -0.5f, 0.0f,	0.0f, 0.75f, 0.0f, 1.0f, //BOTTOM RIGHT (V2)
			 0.12f, 0.5f, 0.0f,	1.0f,  0.0f, 1.0f, 1.0f, //TOP RIGHT    (V3)
			-0.5f, -0.5f, 0.0f, 0.0f, 0.75f, 0.0f, 1.0f, //BOTTOM LEFT  (Must be the same as V1)
			-0.12f, 0.5f, 0.0f,	1.0f,  0.0f, 1.0f, 1.0f, //TOP LEFT     (V4)
			 0.12f, 0.5f, 0.0f,	1.0f,  0.0f, 1.0f, 1.0f, //TOP RIGHT    (Must be the same as V3)
		};

		float squareVertexes[6 * 7] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, //BOTTOM LEFT  (V1)
			 0.5f, -0.5f, 0.0f,	0.0f, 0.0f, 1.0f, 1.0f, //BOTTOM RIGHT (V2)
			 0.5f,  0.5f, 0.0f,	0.0f, 0.0f, 1.0f, 1.0f, //TOP RIGHT    (V3)
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, //BOTTOM LEFT  (Must be the same as V1)
			-0.5f,  0.5f, 0.0f,	0.0f, 0.0f, 1.0f, 1.0f, //TOP LEFT     (V4)
			 0.5f,  0.5f, 0.0f,	0.0f, 0.0f, 1.0f, 1.0f  //TOP RIGHT    (Must be the same as V3)
		};

		uint32_t triangleIndices[] = { 0, 1, 2 };
		uint32_t squareIndices[] = { 0, 1, 2, 3, 4, 5 };

		//TRIANGLE ARRAY
		m_VertexArray.reset(VertexArray::Create());

		std::shared_ptr<VertexBuffer> triangleVB;
		triangleVB.reset(VertexBuffer::Create(triangleVertexes, sizeof(triangleVertexes)));
		triangleVB->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(triangleVB);

		std::shared_ptr<IndexBuffer> triangleIB;
		triangleIB.reset(IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(triangleIB);

		//SQUARE ARRAY
		m_SquareVA.reset(VertexArray::Create());

		std::shared_ptr<VertexBuffer> squareVB;
		squareVB.reset(VertexBuffer::Create(squareVertexes, sizeof(squareVertexes)));
		squareVB->SetLayout(layout);
		m_SquareVA->AddVertexBuffer(squareVB);

		std::shared_ptr<IndexBuffer> squareIB;
		squareIB.reset(IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_SquareVA->SetIndexBuffer(squareIB);

		std::string vertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 position;
			layout(location = 1) in vec4 colour;

			uniform mat4 view;

			out vec3 outPos;
			out vec4 outCol;
			
			void main() {
				outPos = position;
				outCol = colour;
				gl_Position = view * vec4(position, 1.0);
			}
		)";

		std::string fragmentSrc = R"(
			#version 330 core

			layout(location = 0) out vec4 colour;

			in vec3 outPos;
			in vec4 outCol;

			void main() {
				colour = vec4(outPos * 0.5 + 0.5, 1.0);
				colour = outCol;
			}
		)";

		m_Shader.reset(new Shader(vertexSrc, fragmentSrc));
	}

	void Render() {
		RenderCommand::SetClearColour({0.1f, 0.1f, 0.1f, 1.0f});
		RenderCommand::Clear();

		Renderer::BeginScene();
		m_Shader->Bind();
		m_Shader->UploadUniformMat4("view", cam.GetViewProjectionMatrix());
		Renderer::Submit(m_SquareVA);
		Renderer::Submit(m_VertexArray);
		Renderer::EndScene();
	}

	void RecieveEvent(Event& e) {
		if (e.GetEventType() == EventType::KeyPressed) {
			KeyPressedEvent& event = (KeyPressedEvent&)e;
			NB_TRACE("{0}", (char)event.GetKeyCode());
		}
	}

private:
	std::shared_ptr<Shader>		  m_Shader;
	std::shared_ptr<VertexArray>  m_VertexArray;
	std::shared_ptr<VertexArray>  m_SquareVA;

	OrthographicCamera cam;
};

Nebula::Application* Nebula::createApplication() {
	return new App;
}