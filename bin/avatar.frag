#version 330 core

#define MAX_WEB_USERS 256

in vec2 uv;
out vec4 color;

uniform vec3 FaceColor;
uniform sampler2D Tex;
uniform float PingTime;
uniform float CurrTime;
uniform int Team;

void main()
{
    vec4 team0Color = vec4(1.0,0.0,0.0,1.0);
    vec4 team1Color = vec4(0.0,0.0,1.0,1.0);

    vec2 p = -1.0 + 2.0*uv;
    float f = 1.0 - step(0.1,length(p)); // the intensity
    vec3 col = vec3(f*FaceColor); // the color
	color = vec4(col, f);

	float max_radius = 0.5;
	float speed = 10.0;
    float dist = length(p);
    float time = (CurrTime - PingTime)*speed;

    if ( (dist > 0.08*time) && (dist < 0.1*time) && (dist < max_radius) ) {
        color = vec4(FaceColor, 1.0);
    } 
}