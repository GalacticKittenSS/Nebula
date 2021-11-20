#pragma once

#include <Nebula.h>
#include "Player.h"
#include "Colour.h"
#include "Pillar.h"

enum class GameState {
	Start, 
	Running, 
	Paused, 
	Dead
};

class Game : public Nebula::Layer {
public:
	Game(): Camera(CreateCamera()) { }

	~Game() { }

	void Attach() override;
	void Detach() override;

	void OnEvent(Nebula::Event& e) override;

	void Update(Nebula::Timestep ts) override;
	void Render() override;
	void ImGuiRender() override;
public:
	uint32_t Score = 0;
	uint32_t HighScore = 0;
private:
	void Start();
	void OnRun(Nebula::Timestep ts);
	void OnPaused();
	void OnReset();
	
	bool CollisionTest();
	Nebula::OrthographicCamera CreateCamera(float AspectRatio = 16.0f / 9.0f);
	bool OnMouseButton(Nebula::MouseButtonPressedEvent& e);
	
	Nebula::OrthographicCamera Camera;
	Player player;
	Colour col = Colour();
	GameState state = GameState::Start;

	float movespeed = 10.0f;

	std::vector<Pillar> pillar;
	uint32_t pillarNo = 9;
	
};