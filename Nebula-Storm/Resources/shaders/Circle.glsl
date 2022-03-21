#type vertex
#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 localPosition;
layout(location = 2) in vec4 colour;
layout(location = 3) in float thickness;
layout(location = 4) in float fade;
layout(location = 5) in int entityID;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

struct VertexOutput
{
	vec3 LocalPosition;
	vec4 Colour;
	float Thickness;
	float Fade;
};

layout (location = 0) out VertexOutput Output;
layout (location = 4) out flat int v_EntityID;
			
void main() {
	Output.LocalPosition = localPosition;
	Output.Colour = colour;
	Output.Thickness = thickness;
	Output.Fade = fade;
	
	v_EntityID = entityID;

	gl_Position = u_ViewProjection * vec4(position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 colour;
layout(location = 1) out int id;

struct VertexOutput
{
	vec3 LocalPosition;
	vec4 Colour;
	float Thickness;
	float Fade;
};

layout (location = 0) in VertexOutput Input;
layout (location = 4) in flat int v_EntityID;

void main() {
	float distance = 1.0 - length(Input.LocalPosition);
	float alpha = smoothstep(0.0, Input.Fade, distance);
	alpha *= smoothstep(Input.Thickness + Input.Fade, Input.Thickness, distance);
	

	if (alpha == 0.0)
		discard;

	colour = Input.Colour;
	colour.a *= alpha;

	id = v_EntityID;
}