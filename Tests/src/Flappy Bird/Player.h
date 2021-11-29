#pragma once

#include <Nebula.h>

struct Player : public Nebula::Sprite {
	Player() : Nebula::Sprite() { }

	Player(Nebula::vec2 position, Nebula::vec2 size, float rotation = 0.0f) : Nebula::Sprite(position, size, rotation) { }

	void Move(Nebula::vec3 speed) {
		this->position += speed;
	}

	void Move(Nebula::vec2 speed) {
		this->position += Nebula::vec3(speed, 0.0f);
	}

	void Update(Nebula::Timestep ts) {
		this->position.y -= this->gravity * ts;
	}

	float gravity = 7.0f;
};