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
    vec2 p = -1.0 + 2.0*uv;
    float f = 1.0 - step(0.1,length(p)); // the intensity
    vec3 col = vec3(f*FaceColor); // the color
	color = vec4(col, f);

	float max_radius = 0.5;
	float speed = 10.0;
    float dist = length(p);
    float time = (CurrTime - PingTime)*speed;

    if(time < 30) {
        float opacity = sin( (time/2.0)*3.14 );
        color += vec4(1.0, 1.0, 1.0, opacity*f);
    }

    if(Team == 0) {
    	if( (dist > 0.3) && (dist < 0.34) ){
    		color = vec4(1.0, 0.0, 0.0, 1.0);
    	}
    } else if (Team == 1) {
    	if( (dist > 0.3) && (dist < 0.34) ){
    		color = vec4(0.0, 0.0, 1.0, 1.0);
    	}
    }
   
    if ( (dist > 0.08*time) && (dist < 0.1*time) && (dist < max_radius) ) {
        color = vec4(FaceColor, 1.0);
    } 
}