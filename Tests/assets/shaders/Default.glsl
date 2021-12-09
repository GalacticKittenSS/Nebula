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
	gl_Position = u_View * vec4(position, 1.0);
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
	vec4 texColor = v_Colour;
	
	switch(int(v_texIndex))
	{
		case 0:  texColor *= texture(u_Textures[0],  v_texCoord * v_tilingFactor); break;
		case 1:  texColor *= texture(u_Textures[1],  v_texCoord * v_tilingFactor); break;
		case 2:  texColor *= texture(u_Textures[2],  v_texCoord * v_tilingFactor); break;
		case 3:  texColor *= texture(u_Textures[3],  v_texCoord * v_tilingFactor); break;
		case 5:  texColor *= texture(u_Textures[5],  v_texCoord * v_tilingFactor); break;
		case 6:  texColor *= texture(u_Textures[6],  v_texCoord * v_tilingFactor); break;
		case 7:  texColor *= texture(u_Textures[7],  v_texCoord * v_tilingFactor); break;
		case 8:  texColor *= texture(u_Textures[8],  v_texCoord * v_tilingFactor); break;
		case 9:  texColor *= texture(u_Textures[9],  v_texCoord * v_tilingFactor); break;
		case 10: texColor *= texture(u_Textures[10], v_texCoord * v_tilingFactor); break;
		case 11: texColor *= texture(u_Textures[11], v_texCoord * v_tilingFactor); break;
		case 12: texColor *= texture(u_Textures[12], v_texCoord * v_tilingFactor); break;
		case 13: texColor *= texture(u_Textures[13], v_texCoord * v_tilingFactor); break;
		case 14: texColor *= texture(u_Textures[14], v_texCoord * v_tilingFactor); break;
		case 15: texColor *= texture(u_Textures[15], v_texCoord * v_tilingFactor); break;
		case 16: texColor *= texture(u_Textures[16], v_texCoord * v_tilingFactor); break;
		case 17: texColor *= texture(u_Textures[17], v_texCoord * v_tilingFactor); break;
		case 18: texColor *= texture(u_Textures[18], v_texCoord * v_tilingFactor); break;
		case 19: texColor *= texture(u_Textures[19], v_texCoord * v_tilingFactor); break;
		case 20: texColor *= texture(u_Textures[20], v_texCoord * v_tilingFactor); break;
		case 21: texColor *= texture(u_Textures[21], v_texCoord * v_tilingFactor); break;
		case 22: texColor *= texture(u_Textures[22], v_texCoord * v_tilingFactor); break;
		case 23: texColor *= texture(u_Textures[23], v_texCoord * v_tilingFactor); break;
		case 24: texColor *= texture(u_Textures[24], v_texCoord * v_tilingFactor); break;
		case 25: texColor *= texture(u_Textures[25], v_texCoord * v_tilingFactor); break;
		case 26: texColor *= texture(u_Textures[26], v_texCoord * v_tilingFactor); break;
		case 27: texColor *= texture(u_Textures[27], v_texCoord * v_tilingFactor); break;
		case 28: texColor *= texture(u_Textures[28], v_texCoord * v_tilingFactor); break;
		case 29: texColor *= texture(u_Textures[29], v_texCoord * v_tilingFactor); break;
		case 30: texColor *= texture(u_Textures[30], v_texCoord * v_tilingFactor); break;
		case 31: texColor *= texture(u_Textures[31], v_texCoord * v_tilingFactor); break;
	}
	colour = texColor;
}