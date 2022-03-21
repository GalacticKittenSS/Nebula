#type vertex
#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in int EntityID;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

struct VertexOutput
{
	vec4 Colour;
};

layout (location = 0) out VertexOutput Output;
layout (location = 1) out flat int v_EntityID;

void main()
{
	Output.Colour = colour;
	v_EntityID = EntityID;

	gl_Position = u_ViewProjection * vec4(position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 Colour;
layout(location = 1) out int id;

struct VertexOutput
{
	vec4 Colour;
};

layout (location = 0) in VertexOutput Input;
layout (location = 1) in flat int v_EntityID;

void main()
{
	Colour = Input.Colour;
	id = v_EntityID;
}