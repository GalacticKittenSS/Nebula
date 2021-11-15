#pragma once

#include <Nebula.h>

class Example1Layer : public Nebula::Layer {
public:
	Example1Layer();

	~Example1Layer();

	void Start();

	//Called Once Per Frame For Rendering
	void Render();

	//Called Once Per Frame takes in Timestep (deltaTime)
	void Update(Nebula::Timestep ts);

	//Is Called On Event
	void OnEvent(Nebula::Event& e);

	bool OnKeyPressed(Nebula::KeyPressedEvent& event);

private:
	Nebula::ShaderLibrary m_ShaderLib;
	Nebula::Ref<Nebula::VertexArray>  m_TriangleVA;
	Nebula::Ref<Nebula::VertexArray>  m_SquareVA;
	Nebula::Ref<Nebula::Texture2D>	  m_Texture;

	glm::vec3 m_SquareColour = { 0.2f, 0.3f, 0.8f };

	Nebula::OrthographicCameraController m_Controller;
};