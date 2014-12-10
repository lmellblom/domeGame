#version 330 core
 
uniform sampler2D Tex;
uniform vec3 BallVec;
uniform vec3 GoalVec;
uniform vec3 PrevGoalVec;
uniform float CurrTime;
uniform float GoalTime;
uniform int StyleIndex;
 
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
     float goal_radius = 0.36;
     float prev_goal_radius = goal_radius;
     float ball_radius = 0.08;
     float interpolator = clamp(CurrTime - GoalTime,0.0,1.0);
     vec3 point_dir = normalize(pos);
     vec3 goal_dir = normalize(GoalVec);
     vec3 ball_dir = normalize(BallVec);
     vec3 prev_goal_dir = normalize(PrevGoalVec);

     float goal_distance = length(point_dir - goal_dir);
     float prev_distance = length(point_dir - prev_goal_dir);


     vec3 col = vec3(0.0);

if(StyleIndex == 1){ //blackhole

     goal_radius *= interpolator;
     prev_goal_radius *= 1.0 - interpolator;
     vec3 point_dir_goal = mix(goal_dir, point_dir, 1.0-1.0/(goal_distance+1.0));
     vec3 point_dir_prev = mix(prev_goal_dir, point_dir, smoothstep(0.0, 4.0*prev_goal_radius, goal_distance));
     point_dir_goal.x += 0.005*CurrTime;
     point_dir_prev.x += 0.005*CurrTime;
     col = vec3( mix(stars(point_dir_prev)*smoothstep(0.0,prev_goal_radius,prev_distance), stars(point_dir_goal)*smoothstep(0.0,goal_radius,goal_distance), interpolator ) );

     float ball_mask = 1.0 - smoothstep(ball_radius-0.01, ball_radius, length(point_dir - ball_dir) );
	 vec3 ball_color = vec3(1.0);
     col = mix(col,ball_color,ball_mask);

     col += smoothstep((goal_radius/2)+0.01, (goal_radius/2)-0.01, goal_distance)*smoothstep((goal_radius/2)-0.01, (goal_radius/2)+0.01, goal_distance);


}
else if(StyleIndex == 2){ //very simple


    float g_i = smoothstep(0.0,1.0,interpolator);
    goal_radius /= 3.0;
    prev_goal_radius /= 3.0;
    float goal_mask = 1.0 - smoothstep((goal_radius-0.001)*g_i, goal_radius*g_i, length(point_dir - goal_dir) );
    goal_mask = clamp(goal_mask,0.0,1.0);
    vec3 goal_color = vec3(1.0,0.0,0.0);
    col = mix(col,goal_color,goal_mask);

    float p_i = 1.0 - smoothstep(0.0,1.0,interpolator);
    float prev_goal_mask = 1.0 - smoothstep((prev_goal_radius-0.001)*p_i, prev_goal_radius*p_i, length(point_dir - prev_goal_dir) );
    prev_goal_mask = clamp(prev_goal_mask,0.0,1.0);
    col = mix(col,goal_color,prev_goal_mask);

    float ball_mask = 1.0 - smoothstep(ball_radius-0.001, ball_radius, length(point_dir - ball_dir) );
    vec3 ball_color = vec3(0.0,1.0,0.0);
    col = mix(col,ball_color,ball_mask);


}
else if(StyleIndex == 3){ //darkroom

    //light ball
    col = vec3(1.0);
    vec3 v = abs(point_dir);
    float largest_dim = max( v.x, max(v.y, v.z) );
    vec3 wall_pos = point_dir/largest_dim;
    vec3 wall_normal = wall_pos * (smoothstep(0.9998,0.9999,wall_pos) + 1.0 - smoothstep(-0.9999,-0.9998,wall_pos));
    wall_normal *= -1.0;

    float light_dist = length(wall_pos - ball_dir);
    vec3 light_dir = normalize(wall_pos - 0.5*ball_dir);
    float ambient_light = 0.001;
    float diffuse_light = clamp( dot(wall_normal, -light_dir), 0.0, 1.0) * clamp( 1.0 - light_dist, 0.0, 1.0);

    //goals
    float noise = 0.3*fbm(wall_pos*10.0);
    vec3 goal_pos = goal_dir/largest_dim;
    vec3 prev_goal_pos = prev_goal_dir/largest_dim;
    float g_i = smoothstep(0.0,1.0,interpolator);
    goal_radius /= 4.0;
    prev_goal_radius /= 4.0;
    float goal_mask = 1.0 - smoothstep((goal_radius-0.1+noise)*g_i, (goal_radius+noise)*g_i, length(goal_pos - wall_pos) );
    goal_mask = clamp(goal_mask,0.0,1.0);
    col = mix(col,vec3(1.0,0.0,0.0),goal_mask);

    float p_i = 1.0 - smoothstep(0.0,1.0,interpolator);
    float prev_goal_mask = 1.0 - smoothstep((prev_goal_radius-0.1+noise)*p_i, (prev_goal_radius+noise)*p_i, length(prev_goal_pos - wall_pos) );
    prev_goal_mask = clamp(prev_goal_mask,0.0,1.0);

    //finish
    col = mix(col,vec3(1.0,0.0,0.0),prev_goal_mask) * (ambient_light + diffuse_light);
    col += 1.0 - smoothstep(0.0,ball_radius, length(point_dir - ball_dir));

}
else if(StyleIndex == 4){ //metaballs

    interpolator *= 2.0;
    float g_i = smoothstep(0.0,1.0,interpolator);
    goal_radius /= 3.0;
    prev_goal_radius /= 3.0;
    ball_radius *= 3.0;
    float goal_mask = 1.0 - smoothstep(0.0, goal_radius*g_i, length(point_dir - goal_dir) );
    goal_mask = clamp(goal_mask,0.0,1.0);
    vec3 goal_color = vec3(1.0,0.0,0.0);
    col = mix(col,goal_color,goal_mask);

    float p_i = 1.0 - smoothstep(0.0,1.0,interpolator);
    float prev_goal_mask = 1.0 - smoothstep(0.0, prev_goal_radius*p_i, length(point_dir - prev_goal_dir) );
    prev_goal_mask = clamp(prev_goal_mask,0.0,1.0);
    col = mix(col,goal_color,prev_goal_mask);

    float ball_mask = 1.0 - smoothstep(0.0, ball_radius, length(point_dir - ball_dir) );
    vec3 ball_color = vec3(0.0,1.0,0.0);
    col = mix(col,ball_color,ball_mask);
    float metaballs = smoothstep(0.7, 0.8, goal_mask + prev_goal_mask + ball_mask);
    col *= metaballs;
}

     color = vec4(col,1.0);//texture(Tex, UV.st);

  //color=vec4(1.0,0.0,0.0,1.0);
}

