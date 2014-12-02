#version 330 core

in vec2 uv;
out vec4 color;

uniform int Team;

void main()
{
    vec4 team0Color = vec4(1.0,0.0,0.0,1.0);
    vec4 team1Color = vec4(0.0,0.0,1.0,1.0);

    vec3 goalColor = vec3 (0.0, 0.0, 0.0); // color fo rthe goal
    vec2 p = -1.0 + 2.0*uv;
    float f = 1.0 - step(0.8,length(p)); // the intensity
    color = vec4(goalColor, f);

    // circle around if we want to have team color
    float dist = length(p);

    // if distance is long enought, check which team , set teamColor right, else the original color
    color = ( (dist>0.9 && dist < 0.97) ? ( Team==0 ? team0Color : team1Color ) : color);

}


