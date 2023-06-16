#version 460 core

layout(location = 0) out vec4 fragColor;
layout(binding = 0) uniform sampler2D tex;

in block
{
	vec2 texcoord;
} In;

void main()
{
	fragColor = texture(tex, In.texcoord);
}