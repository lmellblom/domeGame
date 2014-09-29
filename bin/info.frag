#version 330 core

in vec2 uv;
out vec4 color;

uniform sampler2D Tex;

void main()
{
	color = texture(Tex, uv.st);
}