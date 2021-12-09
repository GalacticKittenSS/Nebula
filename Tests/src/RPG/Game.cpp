#include "Game.h"

using namespace Nebula;

static const char* s_MapTiles =
"WWWWWWWWWWWWWWWWWWWWWWWW"
"WWWWWWWWWWWWWWWWDDDWWWWW"
"WWWWWWWWWWWWWWWWWDDDWWWW"
"WWWWWDDDDDDDDDDDWWWWWWWW"
"WWDDDDDDDDDDDDDDDDDWWWWW"
"WWWDDDDDDWWDDDDDDWWWWWWW"
"WWWDDDDDWWWWWDDDDWWWWWWW"
"WWWDDDDDDWWWDDDDDDDWWWWW"
"WWWWDDDDDDDDDDDDDDDWWWWW"
"WWWDDDDDDDDDDDDDDDDWWWWW"
"WWWDDDDDDDDDDDDDDDDWWWWW"
"WWWDDDDDDDDDDDDDDDDWWWWW"
"WWWDDDDDDDDDDDDDDDDWWWWW"
"WWWWWDDDDDDDDDDDDWWWWWWW"
"WWWWWWWWWDDDDDDWWWWWWWWW"
"WWWWWWWWWWWWWWWWWWWWWWWW";

static const uint32_t s_MapWidth = 24;
static const uint32_t s_MapHeight = strlen(s_MapTiles) / s_MapWidth;

Game::Game() {

}

Game::~Game() {

}

void Game::Attach() {
	Controller = OrthographicCameraController(16.0f / 9.0f, false);
	SpriteSheet = Texture2D::Create("assets/textures/RPG_spriteSheet.png");

	TextureMap['D'] = Nebula::SubTexture2D::CreateFromCoords(SpriteSheet, { 6, 11 }, { 128, 128 });
	TextureMap['W'] = Nebula::SubTexture2D::CreateFromCoords(SpriteSheet, {11, 11 }, { 128, 128 });
																							    
	for (int i = 0; i < 9; i+=3) {
		grass[i + 0] = SubTexture2D::CreateFromCoords(SpriteSheet, { 0, 12.0f - i / 3.0f }, { 128, 128 }, { 1, 1 });
		grass[i + 1] = SubTexture2D::CreateFromCoords(SpriteSheet, { 1, 12.0f - i / 3.0f }, { 128, 128 }, { 1, 1 });
		grass[i + 2] = SubTexture2D::CreateFromCoords(SpriteSheet, { 2, 12.0f - i / 3.0f }, { 128, 128 }, { 1, 1 });
	}
}

void Game::Detach() {

}

void Game::OnEvent(Nebula::Event& e) {
	Controller.OnEvent(e);
}

void Game::Update(Nebula::Timestep ts) {
	Controller.OnUpdate(ts);
}

void Game::Render() {
	Renderer2D::BeginScene(Controller.GetCamera());

	for (uint32_t y = 0; y < s_MapHeight; y++) {
		for (uint32_t x = 0; x < s_MapWidth; x++) {
			char tiletype = s_MapTiles[x + y * s_MapWidth];
			Ref<SubTexture2D> texture;
			
			if (TextureMap.find(tiletype) != TextureMap.end())
				texture = TextureMap[tiletype];
			else
				texture = grass[5];

			Renderer2D::DrawQuad(Sprite({  x - s_MapWidth / 2.0f, s_MapHeight - y - s_MapHeight / 2.0f }, { 1.0f, 1.0f }, 0.0f, texture));
		}
	}

	//Renderer2D::DrawQuad(Sprite({-1.0f, 1.0f }, { 1.0f, 1.0f }, 0.0f, grass[0]));
	//Renderer2D::DrawQuad(Sprite({ 0.0f, 1.0f }, { 1.0f, 1.0f }, 0.0f, grass[1]));
	//Renderer2D::DrawQuad(Sprite({ 1.0f, 1.0f }, { 1.0f, 1.0f }, 0.0f, grass[2]));
	//Renderer2D::DrawQuad(Sprite({-1.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f, grass[3]));
	//Renderer2D::DrawQuad(Sprite({ 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f, grass[4]));
	//Renderer2D::DrawQuad(Sprite({ 1.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f, grass[5]));
	//Renderer2D::DrawQuad(Sprite({-1.0f,-1.0f }, { 1.0f, 1.0f }, 0.0f, grass[6]));
	//Renderer2D::DrawQuad(Sprite({ 0.0f,-1.0f }, { 1.0f, 1.0f }, 0.0f, grass[7]));
	//Renderer2D::DrawQuad(Sprite({ 1.0f,-1.0f }, { 1.0f, 1.0f }, 0.0f, grass[8]));
	Renderer2D::EndScene();
}

void Game::ImGuiRender() {

}