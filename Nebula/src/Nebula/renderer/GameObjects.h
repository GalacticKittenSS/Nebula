#pragma once

#include "Nebula/Maths/Maths.h"

#include "Texture.h"
#include "Shader.h"

namespace Nebula {
	struct Sprite{
		Sprite(): position({ 0, 0, 0 }), size({ 1, 1 }) { }
		
		Sprite(vec2 pos, vec2 size, float rotation = 0.0f):
			position({ pos.x, pos.y, 0.0f }), size(size), rotation(radians(rotation)) { }
		
		Sprite(vec3 pos, vec2 size, float rotation = 0.0f):
			position(pos), size(size), rotation(radians(rotation)) { }

		Sprite(vec2 pos, vec2 size, float rotation, vec4 colour, Ref<Texture2D> texture = nullptr):
			position({ pos.x, pos.y, 0.0f }), size(size), rotation(radians(rotation)), colour(colour), texture(texture) { }
		
		Sprite(vec3 pos, vec2 size, float rotation, vec4 colour, Ref<Texture2D> texture = nullptr):
			position(pos), size(size), rotation(radians(rotation)), colour(colour), texture(texture) { }

		Sprite(vec2 pos, vec2 size, float rotation, Ref<Texture2D> texture) :
			position({ pos.x, pos.y, 0.0f }), size(size), rotation(radians(rotation)), texture(texture) { }

		Sprite(vec3 pos, vec2 size, float rotation, Ref<Texture2D> texture) :
			position(pos), size(size), rotation(radians(rotation)), texture(texture) { }

		vec3 position;
		vec2 size;
		float rotation = 0.0f;

		Ref<Texture2D> texture = nullptr;
		Ref<Shader> shader = nullptr;
		vec4 colour = { 1, 1, 1, 1 };

		//Should only be used by Renderer
		mat4 CalculateMatrix() {
			mat4 transform = translate(mat4(1.0f), this->position)
				* scale(mat4(1.0f), vec3(this->size.x, this->size.y, 0.0f));
			
			if (rotation != 0.0f)
				transform *= rotate(mat4(1.0f), this->rotation, { 0.0f, 0.0f, 1.0f });

			return transform;
		}
	};
}