#pragma once

#include <Nebula.h>

class Pillar {
public:
	Pillar();

	void RecalculatePos();
	void Update(Nebula::Timestep ts);
	void Submit();

	void SetColour(glm::vec4 col);
	void SetOffset(float offset);

	Nebula::Sprite GetTopPillar() { return upper; }
	Nebula::Sprite GetBottomPillar() { return lower; }
private:
	float moveSpeed = 5.0f;
	float minHeight = 3.0f;

	Nebula::Sprite upper, lower;
public:
	bool canScore = true;
};