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

	Nebula::Triangle GetTopPillar() { return upper; }
	Nebula::Triangle GetBottomPillar() { return lower; }
private:
	float moveSpeed = 5.0f;
	float minHeight = 3.0f;

	Nebula::Triangle upper, lower;
public:
	bool canScore = true;
};