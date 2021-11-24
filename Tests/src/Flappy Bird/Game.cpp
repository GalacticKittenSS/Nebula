#include "Game.h"

static bool PointInTri(const glm::vec2& p, glm::vec2& p0, const glm::vec2& p1, const glm::vec2& p2)
{
	float s = p0.y * p2.x - p0.x * p2.y + (p2.y - p0.y) * p.x + (p0.x - p2.x) * p.y;
	float t = p0.x * p1.y - p0.y * p1.x + (p0.y - p1.y) * p.x + (p1.x - p0.x) * p.y;

	if ((s < 0) != (t < 0))
		return false;

	float A = -p1.y * p2.x + p0.y * (p2.x - p1.x) + p0.x * (p1.y - p2.y) + p1.x * p2.y;

	return A < 0 ?
		(s <= 0 && s + t >= A) :
		(s >= 0 && s + t <= A);
}

void Game::Attach() {
	//Init
	Camera = CreateCamera();
	player = Player({ -4.0f, 0.0f }, { 1.0f, 1.0f });
	player.colour = { 0.2f, 0.3f, 0.8f, 1.0f };
	player.shader = Nebula::Shader::Create("assets/shaders/Flappy.glsl");

	for (uint32_t i = 0; i < pillarNo; i++) {
		pillar.push_back(Pillar());
		pillar[i].SetColour(glm::vec4(col.GetRGB(), 1.0f));
		pillar[i].SetOffset((float)(i * 4 + 5));
	}
}

void Game::Detach() {
	
}

void Game::OnEvent(Nebula::Event& e) {
	Nebula::Dispatcher d(e);
	d.Dispatch<Nebula::MouseButtonPressedEvent>(BIND_EVENT(Game::OnMouseButton));
}

void Game::Update(Nebula::Timestep ts) {
	for (uint32_t i = 0; i < pillar.size(); i++)
		pillar[i].SetColour(glm::vec4(col.GetRGB(), 1.0f));
	
	if (state == GameState::Running)
		OnRun(ts);
}

void Game::OnRun(Nebula::Timestep ts) {
	col.Update(ts);

	for (uint32_t i = 0; i < pillar.size(); i++)
		pillar[i].Update(ts);
	
	if (player.position.y > 6.5 || player.position.y < -6.5)
		state = GameState::Dead;

	if (CollisionTest())
		state = GameState::Dead;

	for (auto& p : pillar) {
		if (p.GetBottomPillar().position.x < player.position.x && p.canScore) {
			p.canScore = false;
			Score += 1;
		}
	}

	if (Nebula::Input::IsKeyPressed(NB_ESCAPE))
		state = GameState::Paused;
	
	if (Nebula::Input::IsKeyPressed(NB_SPACE))
		player.Move({ 0.0f, movespeed * ts });
	else
		player.Update(ts);
}

bool Game::CollisionTest()
{
	if (glm::abs(player.position.y) > 8.5f)
		return true;

	glm::vec4 quadVertices[4] = {
		{ -0.5f, -0.5f, 0.0f, 1.0f },
		{  0.5f, -0.5f, 0.0f, 1.0f },
		{  0.5f,  0.5f, 0.0f, 1.0f },
		{ -0.5f,  0.5f, 0.0f, 1.0f }
	};

	const auto& pos = player.position;
	glm::vec4 playerTransformedVerts[4];
	for (int i = 0; i < 4; i++)
	{
		playerTransformedVerts[i] = glm::translate(glm::mat4(1.0f), pos)
			* glm::rotate(glm::mat4(1.0f), glm::radians(player.rotation), { 0.0f, 0.0f, 1.0f })
			* glm::scale(glm::mat4(1.0f), { 1.0f, 1.3f, 1.0f })
			* quadVertices[i];
	}


	glm::vec4 triVertices[3] = {
		{ -0.5f, -0.5f, 0.0f, 1.0f },
		{  0.5f, -0.5f, 0.0f, 1.0f },
		{  0.0f,  0.5f, 0.0f, 1.0f }
	};

	for (auto& p : pillar) {
		glm::vec2 tri[3];

		// Top pillars
		for (int i = 0; i < 3; i++)
		{
			tri[i] = glm::translate(glm::mat4(1.0f), { p.GetTopPillar().position.x, p.GetTopPillar().position.y, 0.0f })
				* glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), { 0.0f, 0.0f, 1.0f })
				* glm::scale(glm::mat4(1.0f), { p.GetTopPillar().size.x, p.GetTopPillar().size.y, 1.0f })
				* triVertices[i];
		}

		for (auto& vert : playerTransformedVerts)
		{
			if (PointInTri({ vert.x, vert.y }, tri[0], tri[1], tri[2]))
				return true;
		}

		// Bottom pillars
		for (int i = 0; i < 3; i++)
		{
			tri[i] = glm::translate(glm::mat4(1.0f), { p.GetBottomPillar().position.x, p.GetBottomPillar().position.y, 0.0f })
				* glm::scale(glm::mat4(1.0f), { p.GetBottomPillar().size.x, p.GetBottomPillar().size.y, 1.0f })
				* triVertices[i];
		}

		for (auto& vert : playerTransformedVerts)
		{
			if (PointInTri({ vert.x, vert.y }, tri[0], tri[1], tri[2]))
				return true;
		}

	}
	return false;
}

void Game::OnReset() {
	//Reset
	player.position.y = 0;

	for (uint32_t i = 0; i < pillar.size(); i++) {
		pillar[i].SetColour(glm::vec4(col.GetRGB(), 1.0f));
		pillar[i].RecalculatePos();
		pillar[i].SetOffset((float)(i * 4 + 5));
	}

	if (Score > HighScore) {
		HighScore = Score;
		NB_TRACE("New High Score: {0}!", HighScore);
	}

	Score = 0;
}

void Game::Render() {
	Nebula::RenderCommand::Clear();

	Nebula::Renderer2D::BeginScene(Camera);
	Nebula::Renderer2D::DrawQuad(Nebula::Sprite({ 0.0f,  0.0f, -0.1f }, { 32.0f, 18.0f }, 0.0f, { 0.2f, 0.2f, 0.2f, 1 }));
	for (uint32_t i = 0; i < pillar.size(); i++) {
		pillar[i].Submit();
	}
	Nebula::Renderer2D::DrawQuad(player);
	Nebula::Renderer2D::DrawQuad(Nebula::Sprite({ 0.0f,  8.0f, 0.1f }, { 32.0f, 2.0f }, 0.0f, glm::vec4(col.GetRGB(), 1.0f)));
	Nebula::Renderer2D::DrawQuad(Nebula::Sprite({ 0.0f, -8.0f, 0.1f }, { 32.0f, 2.0f }, 0.0f, glm::vec4(col.GetRGB(), 1.0f)));
	Nebula::Renderer2D::EndScene();
}

void Game::ImGuiRender() {
}

Nebula::OrthographicCamera Game::CreateCamera(float AspectRatio) {
	float right = 9.0f * AspectRatio;
	float left = -right;
	float top = 16.0f / AspectRatio;
	float bottom = -top;
	
	return Nebula::OrthographicCamera(left, right, bottom, top);
}

bool Game::OnMouseButton(Nebula::MouseButtonPressedEvent& e) {
	if (state != GameState::Running) {
		if (state == GameState::Dead)
			OnReset();
		
		state = GameState::Running;
	}

	return true;
}