    #version 330 core

    #define MAX_WEB_USERS 256

    uniform sampler2D Tex;
    uniform vec3 PingPos[MAX_WEB_USERS];
    uniform float PingTime[MAX_WEB_USERS];
    uniform int PingId[MAX_WEB_USERS];
    uniform float CurrTime;
     
    in vec2 UV;
    in vec3 world_Position;
    out vec4 color;
     
    void main()
    {	
        // The superawesome sky texture!
        // Will be overwritten if statments below is true
	  	 color = texture(Tex, UV.st);
	    /*
        // Variables... ACTIVATED!
        // int i = 0;
        // int id = 1;
        float circle_radius = 1.2f;
        float border = 2.0f;


        // Instead of looping through 256 users we only look at the users that have pinged
        float dist =  sqrt(dot(world_Position, PingPos[1]));
        if(world_Position)
        if ( (dist > (circle_radius+border)) || (dist < (circle_radius-border)) )
            color = vec4(1.0, 0.0, 0.2, 1.0);

            */
        
    }

