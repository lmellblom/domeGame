    #version 330 core
     
    #define MAX_WEB_USERS 256

    uniform sampler2D Tex;
    uniform vec3[MAX_WEB_USERS] PingPos;
    uniform float[MAX_WEB_USERS] PingTime;
    uniform int[MAX_WEB_USERS] PingId;
    uniform float CurrTime;
     
    in vec2 UV;
    out vec4 color;
     
    void main()
    {	
	  	color = texture(Tex, UV.st);
	    
        // This is where the magic happens
        int i = 0;
        while(PingId[i] != -1) {

            if( CurrTime - PingTime[i] < 1 ) {

                color = vec4( PingPos[i].x, PingPos[i].y, PingPos[i].z, 1.0);
            
            }

            i++;
        } 
        
    }

