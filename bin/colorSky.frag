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

        vec3 col = vec3(1.0);
        
        float g_i = smoothstep(0.0,1.0,interpolator);
        float goal_mask = 1.0 - smoothstep((goal_radius-0.001)*g_i, goal_radius*g_i, length(point_dir - goal_dir) );
        goal_mask = clamp(goal_mask,0.0,1.0);
        vec3 goal_color = goal_dir;
        col = mix(col,goal_color,goal_mask);

        float p_i = 1.0 - smoothstep(0.0,1.0,interpolator);
        float prev_goal_mask = 1.0 - smoothstep((prev_goal_radius-0.001)*p_i, prev_goal_radius*p_i, length(point_dir - prev_goal_dir) );
        prev_goal_mask = clamp(prev_goal_mask,0.0,1.0);
        col = mix(col,goal_color,prev_goal_mask);

        float ball_mask = 1.0 - smoothstep(ball_radius-0.001, ball_radius, length(point_dir - ball_dir) );
    	vec3 ball_color = ball_dir.yzx;
        col = mix(col,ball_color,ball_mask);


        color = vec4(col,1.0);//texture(Tex, UV.st);
    }

