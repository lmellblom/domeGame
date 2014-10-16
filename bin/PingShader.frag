    #version 330 core
     
    #define MAX_WEB_USERS 256

    uniform sampler2D Tex;
    uniform float[MAX_WEB_USERS] PosTime;
    uniform float CurrTime;
     
    in vec2 UV;
    out vec4 color;
     
    void main()
    {	
    	float time = CurrTime/100.0;
	  	color = vec4(time, time, time, time);
	    
	    if( PosTime[1] > 0.0f )
		    color = vec4( 0.0, 1.0, 0.0, 1.0);
		else 
		    color = vec4(time, time, time, time);
	    

    	//texture(Tex, UV.st);
    }

