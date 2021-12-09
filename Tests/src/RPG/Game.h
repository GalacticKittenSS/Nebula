#pragma once

#include <Nebula.h>

class Game: public Nebula::Layer {
public:
	Game();
	~Game();

	void Attach() override;
	void Detach() override;

	void OnEvent(Nebula::Event& e) override;

	void Update(Nebula::Timestep ts) override;
	void Render() override;
	void ImGuiRender() override;
private:
	Nebula::OrthographicCameraController Controller;
	Nebula::Ref<Nebula::Texture2D> SpriteSheet;
	Nebula::Sprite BG;
	Nebula::Ref<Nebula::SubTexture2D> grass[9];

	std::unordered_map<char, Nebula::Ref<Nebula::SubTexture2D>> TextureMap;
};