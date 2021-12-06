#type vertex
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in float texIndex;
layout(location = 4) in float tilingFactor;
			
uniform mat4 u_View;

out vec4 v_Colour;
out vec2 v_texCoord;
out float v_texIndex;
out float v_tilingFactor;
			
void main() {
	v_Colour = colour;
	v_texCoord = texCoord;
	v_texIndex = texIndex;
	v_tilingFactor = tilingFactor;
	gl_Position = u_View * vec4(position.x, position.y, 1.0, 1.0);
}

#type fragment
#version 330 core

layout(location = 0) out vec4 colour;

in vec4 v_Colour;
in vec2 v_texCoord;
in float v_texIndex;
in float v_tilingFactor;

uniform sampler2D u_Textures[32];

void main() {
	colour = texture(u_Textures[int(v_texIndex)], v_texCoord * v_tilingFactor) * v_Colour;
}