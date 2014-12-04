#version 330 core
 
uniform sampler2D Tex;
uniform vec3 BallVec;
uniform vec3 GoalVec;
uniform vec3 PrevGoalVec;
uniform float CurrTime;
uniform float GoalTime;
 
in vec2 UV;
in vec3 pos;
out vec4 color;

//NOISE FUNCTIONS
// random/hash function              
float hash(float n)
{
  return fract(cos(n)*41415.92653);
}
 
// 3d noise function
float noise(vec3 x)
{
  vec3 p  = floor(x);
  vec3 f  = smoothstep(0.0, 1.0, fract(x));
  float n = p.x + p.y*57.0 + 113.0*p.z;
 
  return mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
    mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y),
    mix(mix( hash(n+113.0), hash(n+114.0),f.x),
    mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
}

float fbm(vec3 p)
{
  return 0.5*noise(p)
        +0.25*noise(2.0*p)
        +0.125*noise(4.0*p)
        +0.0625*noise(8.0*p);
} 


float stars(vec3 p) 
{
     float s = noise(p*34.0);
    s += noise(p*170.0);
    s += noise(p*370.0);
    s = pow(s,19.0) * 0.00000001;
    return s+0.2*fbm(10.0*p);
}

void main()
{
     float goal_radius = 0.2;
     float prev_goal_radius = goal_radius;
     float ball_radius = 0.15;
     float interpolator = clamp(CurrTime - GoalTime,0.0,1.0);
     vec3 point_dir = normalize(pos);
     vec3 goal_dir = normalize(GoalVec);
     vec3 ball_dir = normalize(BallVec);
     vec3 prev_goal_dir = normalize(PrevGoalVec);

     float goal_distance = length(point_dir - goal_dir);
     float prev_distance = length(point_dir - prev_goal_dir);
     goal_radius *= interpolator;
     prev_goal_radius *= 1.0 - interpolator;
     vec3 point_dir_goal = mix(goal_dir, point_dir, 1.0-1.0/(goal_distance+1.0));
     vec3 point_dir_prev = mix(prev_goal_dir, point_dir, smoothstep(0.0, 4.0*prev_goal_radius, goal_distance));
     point_dir_goal.x += 0.005*CurrTime;
     point_dir_prev.x += 0.005*CurrTime;
     vec3 col = vec3( mix(stars(point_dir_prev)*smoothstep(0.0,prev_goal_radius,prev_distance), stars(point_dir_goal)*smoothstep(0.0,goal_radius,goal_distance), interpolator ) );


     float ball_mask = 1.0 - smoothstep(ball_radius*0.2*(sin(2.0*CurrTime)+2.0), ball_radius, length(point_dir - ball_dir) );
	 vec3 ball_color = vec3(1.0);
     col = mix(col,ball_color,ball_mask);

     color = vec4(col,1.0);//texture(Tex, UV.st);
  //color=vec4(1.0,0.0,0.0,1.0);
}

