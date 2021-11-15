#type vertex
#version 330 core

layout(location = 0) in vec3 position;
			
uniform mat4 u_View;
uniform mat4 u_Transform;
			
void main() {
	gl_Position = u_View * u_Transform * vec4(position, 1.0);
}

#type fragment
#version 330 core

layout(location = 0) out vec4 colour;

uniform vec4 u_Colour;

void main() {
	colour = u_Colour;
}