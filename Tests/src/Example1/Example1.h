#pragma once

#include <Nebula.h>

class Example1Layer : public Nebula::Layer {
public:
	Example1Layer(): 
		//Layer Debug
		Layer("Example Layer") { }
	
	~Example1Layer() = default;

	void Attach();

	//Called Once Per Frame For Rendering
	void Render();

	//Called Once Per Frame takes in Timestep (deltaTime)
	void Update(Nebula::Timestep ts);

	//Is Called On Event
	void OnEvent(Nebula::Event& e);

	bool OnKeyPressed(Nebula::KeyPressedEvent& event);

private:
	Nebula::ShaderLibrary ShaderLib;
	Nebula::Ref<Nebula::Texture2D>	  Texture;

	Nebula::vec3 m_SquareColour = { 0.2f, 0.3f, 0.8f };

	Nebula::OrthographicCameraController Controller;
};