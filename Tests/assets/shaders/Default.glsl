#type vertex
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
			
uniform mat4 u_View;
uniform mat4 u_Transform;

out vec2 v_TexCoord;
			
void main() {
	v_TexCoord = texCoord;
	gl_Position = u_View * u_Transform * vec4(position, 1.0);
}

#type fragment
#version 330 core

layout(location = 0) out vec4 colour;

in vec2 v_TexCoord;

uniform vec4 u_Colour;
uniform float u_Tiling;
uniform sampler2D u_Texture;

void main() {
	colour = texture(u_Texture, v_TexCoord * u_Tiling) * u_Colour;
}