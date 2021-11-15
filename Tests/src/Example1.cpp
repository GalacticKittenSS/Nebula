#include "Example1.h"

Example1Layer::Example1Layer() :
	//Layer Debug
	Layer("Example Layer") ,
	//Set Camera Mat4
	m_Controller(16.0f / 9.0f, true) {
	Start();
}

Example1Layer::~Example1Layer() { }

void Example1Layer::Start() {
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
	m_TriangleVA = Nebula::VertexArray::Create();

	Nebula::Ref<Nebula::VertexBuffer> triangleVB;
	//VertexBuffer::Create(vertices, size) returns a Vertex Buffer based on platform
	triangleVB = Nebula::VertexBuffer::Create(triangleVertexes, sizeof(triangleVertexes));
	//Sets the Vertex Shader Layout
	triangleVB->SetLayout(colourLayout);
	//Adds Vertex Buffer to Vertex Array
	m_TriangleVA->AddVertexBuffer(triangleVB);

	Nebula::Ref<Nebula::IndexBuffer> triangleIB;
	//IndexBuffer::Create(indices, count) returns Index Buffer based on platform
	triangleIB = Nebula::IndexBuffer::Create(triangleIndices, sizeof(triangleIndices) / sizeof(uint32_t));
	//Sets Vertex Array's Index Buffer
	m_TriangleVA->SetIndexBuffer(triangleIB);

	//SQUARE ARRAY
	//Same as TRIANGLE ARRAY
	m_SquareVA = Nebula::VertexArray::Create(); 

	Nebula::Ref<Nebula::VertexBuffer> squareVB;
	squareVB = Nebula::VertexBuffer::Create(squareVertexes, sizeof(squareVertexes));
	squareVB->SetLayout(textureLayout);
	m_SquareVA->AddVertexBuffer(squareVB);

	Nebula::Ref<Nebula::IndexBuffer> squareIB;
	squareIB = Nebula::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
	m_SquareVA->SetIndexBuffer(squareIB);

	//SHADERS
	//Shader::Create() Returns Shader based on platform
	m_ShaderLib.Load("assets/shaders/Texture.glsl");
	m_ShaderLib.Load("assets/shaders/Colour.glsl");
		
	//Texture
	m_Texture = Nebula::Texture2D::Create("assets/textures/cat.png");
	m_ShaderLib.Get("Texture")->Bind();
	m_ShaderLib.Get("Texture")->SetInt("u_Texture", 0);
}

void Example1Layer::Render() {
	//Window Background Colour
	Nebula::RenderCommand::SetClearColour({ 0.1f, 0.1f, 0.1f, 1.0f });
	Nebula::RenderCommand::Clear();

	Nebula::Renderer::BeginScene(m_Controller.GetCamera());

	//Scale (1/10)
	glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
		
	//Textured Square
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
	Nebula::Renderer::Submit(m_ShaderLib.Get("Texture"), m_SquareVA, transform);

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
			Nebula::Renderer::Submit(m_ShaderLib.Get("Texture"), m_SquareVA, transform);
		}
	}

	//Render Triangle
	//Nebula::Renderer::Submit(m_ShaderLib.Get("Colour"), m_TriangleVA);

	Nebula::Renderer::EndScene();
}

void Example1Layer::Update(Nebula::Timestep ts) {
	m_Controller.OnUpdate(ts);
}

void Example1Layer::OnEvent(Nebula::Event& e) {
	m_Controller.OnEvent(e);

	Nebula::Dispatcher dispatcher(e);
	//Calls Client Made Functions on Event
	dispatcher.Dispatch<Nebula::KeyPressedEvent>(BIND_EVENT(Example1Layer::OnKeyPressed));		//Call When Key is Pressed
}

bool Example1Layer::OnKeyPressed(Nebula::KeyPressedEvent& event) {
	//NB_TRACE("Key {0} was pressed!", (char)event.GetKeyCode());
	return false;
}