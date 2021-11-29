#include "Pillar.h"

Pillar::Pillar() {
	upper = Nebula::Sprite();
	upper.shader = Nebula::Shader::Create("assets/shaders/Flappy.glsl");
	upper.rotation = Nebula::radians(180.0f);

	lower = Nebula::Sprite();
	lower.shader = Nebula::Shader::Create("assets/shaders/Flappy.glsl");
	
	RecalculatePos();
}

void Pillar::RecalculatePos() {
	lower.size.x = 4;
	upper.size.x = 4;
	lower.size.y = (float)(rand() % 10);

	if (lower.size.y < 3.0f)
		lower.size.y = 3.0f;

	float gap = rand() % 30 / 10.0f + 5.0f;
	upper.size.y = 16 - gap - lower.size.y;

	upper.position.y = 7.0f - upper.size.y / 2;
	upper.position.x = 16.0f + upper.size.x / 2;

	lower.position.y = -7.0f + lower.size.y / 2;
	lower.position.x = 16.0f + upper.size.x / 2;

	canScore = true;
}

void Pillar::Update(Nebula::Timestep ts) {
	upper.position.x -= this->moveSpeed * ts;
	lower.position.x -= this->moveSpeed * ts;

	if (lower.position.x < -16.0f - 2) {
		RecalculatePos();
	}
}

void Pillar::Submit() {
	Nebula::Renderer2D::DrawTriangle(upper);
	Nebula::Renderer2D::DrawTriangle(lower);
}

void Pillar::SetColour(Nebula::vec4 col) {
	upper.colour = col;
	lower.colour = col;
}

void Pillar::SetOffset(float offset) {
	lower.position.x = offset;
	upper.position.x = offset;
}