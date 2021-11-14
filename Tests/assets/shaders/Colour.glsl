#type vertex
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 colour;
			
uniform mat4 view;
uniform mat4 transform;

out vec4 vColour;
			
void main() {
	vColour = vec4(colour, 1.0f);
	gl_Position = view * transform * vec4(position, 1.0);
}

#type fragment
#version 330 core

layout(location = 0) out vec4 colour;

in vec4 v_Colour;

void main() {
	colour = v_Colour;
}