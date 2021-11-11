#include <Nebula.h>

class ExampleLayer : public Nebula::Layer {
public:
	ExampleLayer():
		m_Camera(-1.6f, 1.6f, -0.9f, 0.9f), 
		Layer("Example Layer") { 
		Start(); 
	}

	~ExampleLayer() { }

	void Start() {
		Nebula::BufferLayout layout = {
			{Nebula::ShaderDataType::Float3, "position"},
			{Nebula::ShaderDataType::Float4, "colour"}
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
		m_VertexArray.reset(Nebula::VertexArray::Create());

		std::shared_ptr<Nebula::VertexBuffer> triangleVB;
		triangleVB.reset(Nebula::VertexBuffer::Create(triangleVertexes, sizeof(triangleVertexes)));
		triangleVB->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(triangleVB);

		std::shared_ptr<Nebula::IndexBuffer> triangleIB;
		triangleIB.reset(Nebula::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(triangleIB);

		//SQUARE ARRAY
		m_SquareVA.reset(Nebula::VertexArray::Create());

		std::shared_ptr<Nebula::VertexBuffer> squareVB;
		squareVB.reset(Nebula::VertexBuffer::Create(squareVertexes, sizeof(squareVertexes)));
		squareVB->SetLayout(layout);
		m_SquareVA->AddVertexBuffer(squareVB);

		std::shared_ptr<Nebula::IndexBuffer> squareIB;
		squareIB.reset(Nebula::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
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

		m_Shader.reset(new Nebula::Shader(vertexSrc, fragmentSrc));
	}

	void Render() {
		Update();

		Nebula::RenderCommand::SetClearColour({ 0.1f, 0.1f, 0.1f, 1.0f });
		Nebula::RenderCommand::Clear();

		Nebula::Renderer::BeginScene(m_Camera);
		Nebula::Renderer::Submit(m_Shader, m_SquareVA);
		Nebula::Renderer::Submit(m_Shader, m_VertexArray);
		Nebula::Renderer::EndScene();
	}

	void Update() {
		if (Nebula::Input::IsKeyPressed(NB_W))
			m_Camera.SetPosition({ m_Camera.GetPosition().x, m_Camera.GetPosition().y + 0.005f, 0.0f });

		if (Nebula::Input::IsKeyPressed(NB_S))
			m_Camera.SetPosition({ m_Camera.GetPosition().x, m_Camera.GetPosition().y - 0.005f, 0.0f });

		if (Nebula::Input::IsKeyPressed(NB_D))
			m_Camera.SetPosition({ m_Camera.GetPosition().x + 0.005f, m_Camera.GetPosition().y, 0.0f });

		if (Nebula::Input::IsKeyPressed(NB_A))
			m_Camera.SetPosition({ m_Camera.GetPosition().x - 0.005f, m_Camera.GetPosition().y, 0.0f });
	}

	void OnEvent(Nebula::Event& e) {
		Nebula::Dispatcher dispatcher(e);
		dispatcher.Dispatch<Nebula::KeyPressedEvent>(BIND_EVENT(ExampleLayer::OnKeyPressedEvent));
	}

	bool OnKeyPressedEvent(Nebula::KeyPressedEvent& event) {
		NB_TRACE("Key {0} was pressed!", (char)event.GetKeyCode());
		return false;
	}
private:
	std::shared_ptr<Nebula::Shader>		  m_Shader;
	std::shared_ptr<Nebula::VertexArray>  m_VertexArray;
	std::shared_ptr<Nebula::VertexArray>  m_SquareVA;

	Nebula::OrthographicCamera m_Camera;
};

class App : public Nebula::Application {
public:
	App() {
		PushLayer(new ExampleLayer());
	}

	~App() { }
};

Nebula::Application* Nebula::createApplication() {
	return new App;
}