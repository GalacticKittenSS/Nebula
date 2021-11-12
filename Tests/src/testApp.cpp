#include <Nebula.h>

class ExampleLayer : public Nebula::Layer {
public:
	ExampleLayer() :
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

		Nebula::Ref<Nebula::VertexBuffer> triangleVB;
		triangleVB.reset(Nebula::VertexBuffer::Create(triangleVertexes, sizeof(triangleVertexes)));
		triangleVB->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(triangleVB);

		Nebula::Ref<Nebula::IndexBuffer> triangleIB;
		triangleIB.reset(Nebula::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(triangleIB);

		//SQUARE ARRAY
		m_SquareVA.reset(Nebula::VertexArray::Create());

		Nebula::Ref<Nebula::VertexBuffer> squareVB;
		squareVB.reset(Nebula::VertexBuffer::Create(squareVertexes, sizeof(squareVertexes)));
		squareVB->SetLayout(layout);
		m_SquareVA->AddVertexBuffer(squareVB);

		Nebula::Ref<Nebula::IndexBuffer> squareIB;
		squareIB.reset(Nebula::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_SquareVA->SetIndexBuffer(squareIB);

		std::string vertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 position;
			layout(location = 1) in vec4 colour;
			
			uniform mat4 view;
			uniform mat4 transform;

			out vec3 outPos;
			out vec4 outCol;
			
			void main() {
				outPos = position;
				outCol = colour;
				gl_Position = view * transform * vec4(position, 1.0);
			}
		)";

		std::string fragmentSrc = R"(
			#version 330 core

			layout(location = 0) out vec4 colour;

			in vec3 outPos;
			in vec4 outCol;

			void main() {
				colour = outCol;
			}
		)";

		std::string vertColSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 position;
			
			uniform mat4 view;
			uniform mat4 transform;

			out vec3 outPos;
			
			void main() {
				outPos = position;
				gl_Position = view * transform * vec4(position, 1.0);
			}
		)";

		std::string fragColSrc = R"(
			#version 330 core

			layout(location = 0) out vec4 colour;

			uniform vec3 col;

			in vec3 outPos;

			void main() {
				colour = vec4(col, 1.0);
			}
		)";

		m_Shader.reset(Nebula::Shader::Create(vertexSrc, fragmentSrc));
		m_SquareShader.reset(Nebula::Shader::Create(vertColSrc, fragColSrc));
	}

	void Render() {
		Nebula::RenderCommand::SetClearColour({ 0.1f, 0.1f, 0.1f, 1.0f });
		Nebula::RenderCommand::Clear();

		Nebula::Renderer::BeginScene(m_Camera);

		std::dynamic_pointer_cast<Nebula::OpenGL_Shader>(m_SquareShader)->Bind();
		std::dynamic_pointer_cast<Nebula::OpenGL_Shader>(m_SquareShader)->UploadUniformFloat3("col", m_SquareColour);

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
		glm::vec4 red(0.8f, 0.2f, 0.3f, 1.0f);
		glm::vec4 blue(0.2f, 0.3f, 0.8f, 1.0f);
		
		for (int y = -10; y < 10; y++) {
			for (int x = -10; x < 10; x++) {
				glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				Nebula::Renderer::Submit(m_SquareShader, m_SquareVA, transform);
			}
		}

		Nebula::Renderer::Submit(m_Shader, m_VertexArray);
		Nebula::Renderer::EndScene();

		//IMGUI
		ImGui::Begin("Settings");
		ImGui::ColorEdit3("Square Colour", glm::value_ptr(m_SquareColour));
		ImGui::End();
	}

	void Update(Nebula::Timestep ts) {
		glm::vec3 position = m_Camera.GetPosition();
		float rotation = m_Camera.GetRotation();

		// CAMERA
		if (Nebula::Input::IsKeyPressed(NB_W))
			position.y += moveSpeed * ts;

		if (Nebula::Input::IsKeyPressed(NB_S))
			position.y
			-= moveSpeed * ts;

		if (Nebula::Input::IsKeyPressed(NB_D))
			position.x += moveSpeed * ts;

		if (Nebula::Input::IsKeyPressed(NB_A))
			position.x -= moveSpeed * ts;

		m_Camera.SetPosition(position);
		m_Camera.SetRotation(rotation);
	}

	void OnEvent(Nebula::Event& e) {
		Nebula::Dispatcher dispatcher(e);
		dispatcher.Dispatch<Nebula::KeyPressedEvent>(BIND_EVENT(ExampleLayer::OnKeyPressed));
		dispatcher.Dispatch<Nebula::WindowResizeEvent>(BIND_EVENT(ExampleLayer::OnWindowResize));
	}

	bool OnKeyPressed(Nebula::KeyPressedEvent& event) {
		//NB_TRACE("Key {0} was pressed!", (char)event.GetKeyCode());
		return false;
	}

	bool OnWindowResize(Nebula::WindowResizeEvent& event) {
		NB_TRACE(event);
		return false;
	}

private:
	Nebula::Ref<Nebula::Shader>		  m_Shader;
	Nebula::Ref<Nebula::Shader>		  m_SquareShader;
	Nebula::Ref<Nebula::VertexArray>  m_VertexArray;
	Nebula::Ref<Nebula::VertexArray>  m_SquareVA;

	glm::vec3 m_SquareColour = { 0.2f, 0.3f, 0.8f };

	Nebula::OrthographicCamera m_Camera;

	float moveSpeed = 5.0f;
	float rotSpeed = 180.0f;
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