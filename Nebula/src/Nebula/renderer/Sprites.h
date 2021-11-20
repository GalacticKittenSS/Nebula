#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Texture.h"

namespace Nebula {
	struct Quad{
		Quad(): position({ 0, 0, 0 }), size({ 1, 1 }) { }
		
		Quad(glm::vec2 pos, glm::vec2 size, float rotation = 0.0f):
			position({ pos.x, pos.y, 0.0f }), size(size), rotation(glm::radians(rotation)) { }
		
		Quad(glm::vec3 pos, glm::vec2 size, float rotation = 0.0f): 
			position(pos), size(size), rotation(glm::radians(rotation)) { }

		Quad(glm::vec2 pos, glm::vec2 size, float rotation, glm::vec4 colour, Ref<Texture2D> texture = nullptr): 
			position({ pos.x, pos.y, 0.0f }), size(size), rotation(glm::radians(rotation)), colour(colour), texture(texture) { }
		
		Quad(glm::vec3 pos, glm::vec2 size, float rotation, glm::vec4 colour, Ref<Texture2D> texture = nullptr): 
			position(pos), size(size), rotation(glm::radians(rotation)), colour(colour), texture(texture) { }

		Quad(glm::vec2 pos, glm::vec2 size, float rotation, Ref<Texture2D> texture) :
			position({ pos.x, pos.y, 0.0f }), size(size), rotation(glm::radians(rotation)), texture(texture) { }

		Quad(glm::vec3 pos, glm::vec2 size, float rotation, Ref<Texture2D> texture) :
			position(pos), size(size), rotation(glm::radians(rotation)), texture(texture) { }

		glm::vec3 position;
		glm::vec2 size;
		float rotation = 0.0f;

		Ref<Texture2D> texture = nullptr;
		glm::vec4 colour = {1, 1, 1, 1};

		//Should only be used by Renderer
		glm::mat4 CalculateMatrix() {
			glm::mat4 transform = glm::translate(glm::mat4(1.0f), this->position)
				* glm::scale(glm::mat4(1.0f), { this->size.x, this->size.y, 0.0f });
			
			if (rotation != 0.0f)
				transform *= glm::rotate(glm::mat4(1.0f), this->rotation, { 0.0f, 0.0f, 1.0f });

			return transform;
		}
	}; 
	
	struct Triangle {
		Triangle() : position({ 0, 0, 0 }), size({ 1, 1 }) { }

		Triangle(glm::vec2 pos, glm::vec2 size, float rotation = 0.0f) :
			position({ pos.x, pos.y, 0.0f }), size(size), rotation(glm::radians(rotation)) { }

		Triangle(glm::vec3 pos, glm::vec2 size, float rotation = 0.0f) :
			position(pos), size(size), rotation(glm::radians(rotation)) { }

		Triangle(glm::vec2 pos, glm::vec2 size, float rotation, glm::vec4 colour, Ref<Texture2D> texture = nullptr) :
			position({ pos.x, pos.y, 0.0f }), size(size), rotation(glm::radians(rotation)), colour(colour), texture(texture) { }

		Triangle(glm::vec3 pos, glm::vec2 size, float rotation, glm::vec4 colour, Ref<Texture2D> texture = nullptr) :
			position(pos), size(size), rotation(glm::radians(rotation)), colour(colour), texture(texture) { }

		Triangle(glm::vec2 pos, glm::vec2 size, float rotation, Ref<Texture2D> texture) :
			position({ pos.x, pos.y, 0.0f }), size(size), rotation(glm::radians(rotation)), texture(texture) { }

		Triangle(glm::vec3 pos, glm::vec2 size, float rotation, Ref<Texture2D> texture) :
			position(pos), size(size), rotation(glm::radians(rotation)), texture(texture) { }

		glm::vec3 position;
		glm::vec2 size;
		float rotation = 0.0f;

		Ref<Texture2D> texture = nullptr;
		glm::vec4 colour = { 1, 1, 1, 1 };

		//Should only be used by Renderer
		glm::mat4 CalculateMatrix() {
			glm::mat4 transform = glm::translate(glm::mat4(1.0f), this->position)
				* glm::scale(glm::mat4(1.0f), { this->size.x, this->size.y, 0.0f });

			if (rotation != 0.0f)
				transform *= glm::rotate(glm::mat4(1.0f), this->rotation, { 0.0f, 0.0f, 1.0f });

			return transform;
		}
	};
}