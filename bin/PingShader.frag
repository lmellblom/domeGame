    #version 330 core
     
    #define MAX_WEB_USERS 256

    uniform sampler2D Tex;
    uniform vec3[MAX_WEB_USERS] PingPos;
    uniform float[MAX_WEB_USERS] PingTime;
    uniform float CurrTime;
     
    in vec2 UV;
    out vec4 color;
     
    void main()
    {	
    	float time = CurrTime/100.0;
	  	color = vec4(time, time, time, time);
	    
	    if( PingTime[1] > 0.0f && ( CurrTime - PingTime[1] < 1 ) )
		    color = vec4( PingPos[1].x, PingPos[1].y, PingPos[1].z, 1.0);
		else 
		    color = vec4(time, time, time, time);
	    

    	//texture(Tex, UV.st);
    }

