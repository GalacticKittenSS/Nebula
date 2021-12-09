#pragma once

#include "Nebula/Maths/Maths.h"

#include "Texture.h"
#include "Shader.h"

namespace Nebula {
	struct Sprite{
		Sprite() { }
		
		Sprite(vec2 pos, vec2 size, float rotation = 0.0f):
			position({ pos.x, pos.y, 0.0f }), size(size), rotation(radians(rotation)) { }
		
		Sprite(vec3 pos, vec2 size, float rotation = 0.0f):
			position(pos), size(size), rotation(radians(rotation)) { }

		//TEXTURES

		Sprite(vec2 pos, vec2 size, float rotation, vec4 colour, Ref<Texture2D> texture = nullptr):
			position({ pos.x, pos.y, 0.0f }), size(size), rotation(radians(rotation)), colour(colour), texture(texture) { }
		
		Sprite(vec3 pos, vec2 size, float rotation, vec4 colour, Ref<Texture2D> texture = nullptr):
			position(pos), size(size), rotation(radians(rotation)), colour(colour), texture(texture) { }

		Sprite(vec2 pos, vec2 size, float rotation, Ref<Texture2D> texture) :
			position({ pos.x, pos.y, 0.0f }), size(size), rotation(radians(rotation)), texture(texture) { }

		Sprite(vec3 pos, vec2 size, float rotation, Ref<Texture2D> texture) :
			position(pos), size(size), rotation(radians(rotation)), texture(texture) { }

		//SUB TEXTURES

		Sprite(vec2 pos, vec2 size, float rotation, vec4 colour, Ref<SubTexture2D> texture) :
			position({ pos.x, pos.y, 0.0f }), size(size), rotation(radians(rotation)), colour(colour), 
			texture(texture->GetTexture()) {
			const vec2* coords = texture->GetTextureCoords();
			texCoords[0] = coords[0];
			texCoords[1] = coords[1];
			texCoords[2] = coords[2];
			texCoords[3] = coords[3];
		}

		Sprite(vec3 pos, vec2 size, float rotation, vec4 colour, Ref<SubTexture2D> texture) :
			position(pos), size(size), rotation(radians(rotation)), colour(colour), 
			texture(texture->GetTexture()) {
			const vec2* coords = texture->GetTextureCoords();
			texCoords[0] = coords[0];
			texCoords[1] = coords[1];
			texCoords[2] = coords[2];
			texCoords[3] = coords[3];
		}

		Sprite(vec2 pos, vec2 size, float rotation, Ref<SubTexture2D> texture) :
			position({ pos.x, pos.y, 0.0f }), size(size), rotation(radians(rotation)), 
			texture(texture->GetTexture()) {
			const vec2* coords = texture->GetTextureCoords();
			texCoords[0] = coords[0];
			texCoords[1] = coords[1];
			texCoords[2] = coords[2];
			texCoords[3] = coords[3];
		}

		Sprite(vec3 pos, vec2 size, float rotation, Ref<SubTexture2D> texture) :
			position(pos), size(size), rotation(radians(rotation)), 
			texture(texture->GetTexture()) {
			const vec2* coords = texture->GetTextureCoords();
			texCoords[0] = coords[0];
			texCoords[1] = coords[1];
			texCoords[2] = coords[2];
			texCoords[3] = coords[3];
		}

		vec3 position = { 0, 0, 0 };
		vec2 size = { 1, 1 };
		float rotation = 0.0f;

		vec2 texCoords[4] = {
			{ 0.0f, 0.0f },
			{ 1.0f, 0.0f },
			{ 1.0f, 1.0f },
			{ 0.0f, 1.0f }
		};

		Ref<Texture2D> texture = nullptr;
		Ref<Shader> shader = nullptr;
		vec4 colour = { 1, 1, 1, 1 };
	};
}