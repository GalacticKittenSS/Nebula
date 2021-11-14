#include <Nebula.h>

class ExampleLayer : public Nebula::Layer {
public:
	ExampleLayer() :
		//Layer Debug
		Layer("Example Layer") ,
		//Set Camera Mat4
		m_Camera(-1.6f, 1.6f, -0.9f, 0.9f) {
		Start();
	}

	~ExampleLayer() { }

	void Start() {
		//Layout of Colour Shader
		Nebula::BufferLayout colourLayout = {
			{Nebula::ShaderDataType::Float3, "position"},
			{Nebula::ShaderDataType::Float3, "colour"}
		};

		//Layout of Texture Shader
		Nebula::BufferLayout textureLayout = {
			{Nebula::ShaderDataType::Float3, "position"},
			{Nebula::ShaderDataType::Float2, "texCoord"}
		};

		
		//Triangle vertices [Each Vertex has a Vec3 Pos and a Vec3 Colour]
		float triangleVertexes[3 * (3 + 3)] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.75f, 0.0f, //BOTTOM LEFT  (V1)
			 0.5f, -0.5f, 0.0f,	0.0f, 0.75f, 0.0f, //BOTTOM RIGHT (V2)
			 0.0f,  0.5f, 0.0f,	1.0f,  0.0f, 1.0f  //TOP		  (V3)
		};

		//Square vertices for Texture [Each Vertex has a Vec3 Pos and a Vec2 Tex Coord]
		float squareVertexes[4 * (3 + 2)] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, //BOTTOM LEFT  (V1)
			 0.5f, -0.5f, 0.0f,	1.0f, 0.0f, //BOTTOM RIGHT (V2)
			 0.5f,  0.5f, 0.0f,	1.0f, 1.0f, //TOP RIGHT    (V3)
			-0.5f,  0.5f, 0.0f,	0.0f, 1.0f	//TOP LEFT     (V4)
		};

		//Order of Vertices
		uint32_t triangleIndices[] =		{ 0, 1, 2 };
		uint32_t squareIndices[] =			{ 0, 1, 2, 2, 3, 0 };

		//TRIANGLE ARRAY
		//VertexArray::Create() returns a Vertex Array for the Triangle based on platform
		m_TriangleVA.reset(Nebula::VertexArray::Create());

		Nebula::Ref<Nebula::VertexBuffer> triangleVB;
		//VertexBuffer::Create(vertices, size) returns a Vertex Buffer based on platform
		triangleVB.reset(Nebula::VertexBuffer::Create(triangleVertexes, sizeof(triangleVertexes)));
		//Sets the Vertex Shader Layout
		triangleVB->SetLayout(colourLayout);
		//Adds Vertex Buffer to Vertex Array
		m_TriangleVA->AddVertexBuffer(triangleVB);

		Nebula::Ref<Nebula::IndexBuffer> triangleIB;
		//IndexBuffer::Create(indices, count) returns Index Buffer based on platform
		triangleIB.reset(Nebula::IndexBuffer::Create(triangleIndices, sizeof(triangleIndices) / sizeof(uint32_t)));
		//Sets Vertex Array's Index Buffer
		m_TriangleVA->SetIndexBuffer(triangleIB);

		//SQUARE ARRAY
		//Same as TRIANGLE ARRAY
		m_SquareVA.reset(Nebula::VertexArray::Create()); 

		Nebula::Ref<Nebula::VertexBuffer> squareVB;
		squareVB.reset(Nebula::VertexBuffer::Create(squareVertexes, sizeof(squareVertexes)));
		squareVB->SetLayout(textureLayout);
		m_SquareVA->AddVertexBuffer(squareVB);

		Nebula::Ref<Nebula::IndexBuffer> squareIB;
		squareIB.reset(Nebula::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_SquareVA->SetIndexBuffer(squareIB);

		//SHADERS
		//Shader::Create() Returns Shader based on platform
		m_TextureShader.reset(Nebula::Shader::Create("assets/shaders/Texture.glsl"));
		m_ColourShader.reset(Nebula::Shader::Create("assets/shaders/Colour.glsl"));
		
		//Texture
		m_Texture = Nebula::Texture2D::Create("assets/textures/rgba.png");
		std::dynamic_pointer_cast<Nebula::OpenGL_Shader>(m_TextureShader)->Bind();
		std::dynamic_pointer_cast<Nebula::OpenGL_Shader>(m_TextureShader)->UploadUniformInt("u_Texture", 0);
	}

	//Called Once Per Frame For Rendering
	void Render() {
		//Window Background Colour
		Nebula::RenderCommand::SetClearColour({ 0.1f, 0.1f, 0.1f, 1.0f });
		Nebula::RenderCommand::Clear();

		Nebula::Renderer::BeginScene(m_Camera);

		//Scale (1/10)
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
		
		//Bind the Texture to all following Cubes
		m_Texture->Bind();
		//Create 100 Cubes
		for (int y = -10; y < 10; y++) {
			for (int x = -10; x < 10; x++) {
				//Get Position with 0.01 gap
				glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
				//Position as Mat4 * Scale
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				//Render Cube Immediately
				Nebula::Renderer::Submit(m_TextureShader, m_SquareVA, transform);
			}
		}

		//Render Triangle
		//Nebula::Renderer::Submit(m_ColourShader, m_TriangleVA);

		//Textured Square
		m_Texture->Bind();
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
		Nebula::Renderer::Submit(m_TextureShader, m_SquareVA, transform);

		Nebula::Renderer::EndScene();

		//IMGUI
		/*
		//Change Colour of cubes
		ImGui::Begin("Settings"); //Window Name
		ImGui::ColorEdit3("Square Colour", glm::value_ptr(m_SquareColour)); //Edit Variable m_SquareColour
		ImGui::End();*/
	}

	//Called Once Per Frame takes in Timestep (deltaTime)
	void Update(Nebula::Timestep ts) {
		//Get Position and Rotation of Camera
		glm::vec3 position = m_Camera.GetPosition();
		float rotation = m_Camera.GetRotation();

		//Increment Camera Position when Key is Pressed
		if (Nebula::Input::IsKeyPressed(NB_W))
			position.y += moveSpeed * ts;		//UP

		if (Nebula::Input::IsKeyPressed(NB_S))
			position.y -= moveSpeed * ts;		//DOWN

		if (Nebula::Input::IsKeyPressed(NB_D))
			position.x += moveSpeed * ts;		//RIGHT

		if (Nebula::Input::IsKeyPressed(NB_A))
			position.x -= moveSpeed * ts;		//LEFT

		//Set Camera Position and Rotation to Incremented Value
		m_Camera.SetPosition(position);
		m_Camera.SetRotation(rotation);
	}

	//Is Called On Event
	void OnEvent(Nebula::Event& e) {
		Nebula::Dispatcher dispatcher(e);
		//Calls Client Made Functions on Event
		dispatcher.Dispatch<Nebula::KeyPressedEvent>(BIND_EVENT(ExampleLayer::OnKeyPressed));		//Call When Key is Pressed
		dispatcher.Dispatch<Nebula::WindowResizeEvent>(BIND_EVENT(ExampleLayer::OnWindowResize));	//Call When Window is Resized
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
	Nebula::Ref<Nebula::Shader>		  m_ColourShader;
	Nebula::Ref<Nebula::Shader>		  m_TextureShader;
	Nebula::Ref<Nebula::VertexArray>  m_TriangleVA;
	Nebula::Ref<Nebula::VertexArray>  m_SquareVA;
	Nebula::Ref<Nebula::Texture2D>	  m_Texture;

	glm::vec3 m_SquareColour = { 0.2f, 0.3f, 0.8f };

	Nebula::OrthographicCamera m_Camera;

	float moveSpeed = 5.0f;
	float rotSpeed = 180.0f;
};

//Application Class (Calls Layers)
class App : public Nebula::Application {
public:
	App() {
		//Add Layer to List for Rendering and Startup
		PushLayer(new ExampleLayer());
	}

	~App() { }
};

//Returns App to Nebula Engine on Startup
Nebula::Application* Nebula::createApplication() {
	return new App;
}