#version 330 core

in vec2 uv;
out vec4 color;

uniform vec3 FaceColor;
uniform sampler2D Tex;

void main()
{
    vec2 p = -1.0 + 2.0*uv;
    float f = 1.0 - smoothstep(0.7,1.0,length(p)); // the intensity
    vec3 col = vec3(f*FaceColor); // the color
	color = vec4(col, f);
}