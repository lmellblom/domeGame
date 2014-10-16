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

        // Variables... ACTIVATED!
        float max_radius = 0.1;

        for(int i = 0; i < 20; i++) {

                float time = (CurrTime - PingTime[1])*10;
                vec3 normalizedPos = normalize(PingPos[i]);
                vec3 normalizedWorldPos = normalize(world_Position);

                float dist = length(normalizedWorldPos - normalizedPos);

                if ( (dist > 0.03*time) && (dist < 0.04*time) && (dist < max_radius) ) {
                      color = vec4(1.0, 1.0, 1.0, 1.0);
                }  
            
             
        }
        
    }

