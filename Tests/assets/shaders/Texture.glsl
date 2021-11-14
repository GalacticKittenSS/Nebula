#type vertex
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
			
uniform mat4 view;
uniform mat4 transform;

out vec2 v_TexCoord;
			
void main() {
	v_TexCoord = texCoord;
	gl_Position = view * transform * vec4(position, 1.0);
}

#type fragment
#version 330 core

layout(location = 0) out vec4 colour;

in vec2 v_TexCoord;

uniform sampler2D u_Texture;

void main() {
	colour = texture(u_Texture, v_TexCoord);
}