#pragma once

#include <Nebula.h>

struct Player : public Nebula::Quad {
	Player() : Nebula::Quad() { }

	Player(glm::vec2 position, glm::vec2 size, float rotation = 0.0f): Nebula::Quad(position, size, rotation) { }

	void Move(glm::vec3 speed) {
		this->position += speed;
	}

	void Move(glm::vec2 speed) {
		this->position += glm::vec3(speed, 0.0f);
	}

	void Update(Nebula::Timestep ts) {
		this->position.y -= this->gravity * ts;
	}

	float gravity = 7.0f;
};