#version 330 core

in vec2 uv;
out vec4 color;

uniform vec3 FaceColor;
uniform sampler2D Tex;

void main()
{
    vec2 p = -1.0 + 2.0*uv;
    float f = 1.0 - smoothstep(0.0,1.0,length(p));
    vec3 col = vec3(f);
	color = vec4(col, 1.0);// * texture(Tex, uv.st);
}