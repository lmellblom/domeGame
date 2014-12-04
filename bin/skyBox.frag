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

//noise https://github.com/ashima/webgl-noise
vec3 mod289(vec3 x);
vec4 mod289(vec4 x);
vec4 permute(vec4 x);
vec4 taylorInvSqrt(vec4 r);
float snoise(vec3 v);

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

float noise2(vec3 x)
{
    return 0.5 + 0.5*snoise(x);
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
    return vec3(s)+0.2*fbm(10.0*p);
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
}

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
{ 
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //   x0 = x0 - 0.0 + 0.0 * C.xxx;
  //   x1 = x0 - i1  + 1.0 * C.xxx;
  //   x2 = x0 - i2  + 2.0 * C.xxx;
  //   x3 = x0 - 1.0 + 3.0 * C.xxx;
  vec3 x1 = x0 - i1 + C.xxx;
  vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
  vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
  i = mod289(i); 
  vec4 p = permute( permute( permute( 
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
  float n_ = 0.142857142857; // 1.0/7.0
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
  //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
                                dot(p2,x2), dot(p3,x3) ) );
}