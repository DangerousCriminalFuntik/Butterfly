#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;

out block
{
	vec2 texcoord;
} Out;

uniform mat4 mvpMatrix;

out gl_PerVertex
{
	vec4 gl_Position;
};


void main()
{
	gl_Position = mvpMatrix * vec4(position, 1.0);
	Out.texcoord = texcoord;
}