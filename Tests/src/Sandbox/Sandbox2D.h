#pragma once

#include <Nebula.h>

class Sandbox2D : public Nebula::Layer {
public:
	Sandbox2D();
	~Sandbox2D() = default;

	void Attach() override;
	void Detach() override;

	void Update(Nebula::Timestep ts) override;
	void Render() override;
	
	void ImGuiRender() override;

	void OnEvent(Nebula::Event& e) override;
	bool OnKeyPressed(Nebula::KeyPressedEvent& e);
private:
	Nebula::Ref<Nebula::FrameBuffer> frameBuffer;

	Nebula::OrthographicCameraController Controller;

	Nebula::Timer timer;
};