#include "nbpch.h"
#include "Application.h"
#include "events/Key_Event.h"

#include "Input.h"

#include <glad/glad.h>

namespace Nebula {
	Application* Application::s_Instance = nullptr;

	Application::Application() {
		NB_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT(Application::OnEvent));

		m_ImGui = new ImGuiLayer();
		PushOverlay(m_ImGui);

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

			out vec3 outPos;
			out vec4 outCol;
			
			void main() {
				outPos = position;
				outCol = colour;
				gl_Position = vec4(position, 1.0);
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

	Application::~Application() {

	}

	void Application::run() {
		while (m_Running) {
			glClearColor(0.1f, 0.1f, 0.1f, 1); //RenderCommand::SetClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT); //RenderCommand::Clear();

			//Renderer::BeginScene();

			m_Shader->Bind();

			//Renderer::Submit(m_SquareVA);
			m_SquareVA->Bind();
			glDrawElements(GL_TRIANGLES, m_SquareVA->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);

			//Renderer::Submit(m_VertexArray);
			m_VertexArray->Bind();
			glDrawElements(GL_TRIANGLES, m_VertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);

			//Renderer::EndScene();

			for (Layer* layer : m_LayerStack)
				layer->Update();

			m_ImGui->Begin();
			for (Layer* layer : m_LayerStack)
				layer->Render();
			m_ImGui->End();

			m_Window->Update();
		}
	}

	void Application::PushLayer(Layer* layer) {
		m_LayerStack.PushLayer(layer);
	}

	void Application::PopLayer(Layer* layer) {
		m_LayerStack.PopLayer(layer);
	}
	
	void Application::PushOverlay(Layer* overlay) {
		m_LayerStack.PushOverlay(overlay);
	}

	void Application::PopOverlay(Layer* overlay) {
		m_LayerStack.PopOverlay(overlay);
	}

	void Application::OnEvent(Event& e) {
		if (e.GetEventType() == EventType::KeyPressed) {
			KeyPressedEvent& event = (KeyPressedEvent&)e;
			NB_TRACE("{0}", (char)event.GetKeyCode());
		}

		Dispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT(Application::OnWindowClose));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();) {
			(*--it)->OnEvent(e);

			if (e.Handled)
				break;
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e) {
		m_Running = false;
		return true;
	}
}