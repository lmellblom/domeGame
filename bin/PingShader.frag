    #version 330 core

    #define MAX_WEB_USERS 256

    uniform sampler2D Tex;
    uniform vec3 PingPos[MAX_WEB_USERS];
    uniform float PingTime[MAX_WEB_USERS];
    //uniform int PingId[MAX_WEB_USERS];
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
        float speed = 10.0;
        float time;
        vec3 normalizedPos;
        vec3 normalizedWorldPos;
        float dist;
        bool pingedCheck = true;
        
        // MAX 18 PLAYERS
        for(int i = 1; i < 18; i++) {            
            switch(i){
                case 1:
                    time = (CurrTime - PingTime[1])*speed;
                    normalizedPos = normalize(PingPos[1]);                                  
                    break;
                case 2:
                    time = (CurrTime - PingTime[2])*speed;
                    normalizedPos = normalize(PingPos[2]);                                  
                    break;
                case 3:
                    time = (CurrTime - PingTime[3])*speed;
                    normalizedPos = normalize(PingPos[3]);                                  
                    break;
                case 4:
                    time = (CurrTime - PingTime[4])*speed;
                    normalizedPos = normalize(PingPos[4]);                                  
                    break;
                case 5:
                    time = (CurrTime - PingTime[5])*speed;
                    normalizedPos = normalize(PingPos[5]);                                  
                    break;
                case 6:
                    time = (CurrTime - PingTime[6])*speed;
                    normalizedPos = normalize(PingPos[6]);                                  
                    break;
                case 7:
                    time = (CurrTime - PingTime[7])*speed;
                    normalizedPos = normalize(PingPos[7]);                                  
                    break;
                case 8:
                    time = (CurrTime - PingTime[8])*speed;
                    normalizedPos = normalize(PingPos[8]);                                  
                    break;
                case 9:
                    time = (CurrTime - PingTime[9])*speed;
                    normalizedPos = normalize(PingPos[9]);                                  
                    break;
                case 10:
                    time = (CurrTime - PingTime[10])*speed;
                    normalizedPos = normalize(PingPos[10]);                                  
                    break;
                case 11:
                    time = (CurrTime - PingTime[11])*speed;
                    normalizedPos = normalize(PingPos[11]);                                  
                    break;
                case 12:
                    time = (CurrTime - PingTime[12])*speed;
                    normalizedPos = normalize(PingPos[12]);                                  
                    break;
                case 13:
                    time = (CurrTime - PingTime[13])*speed;
                    normalizedPos = normalize(PingPos[13]);                                  
                    break;
                case 14:
                    time = (CurrTime - PingTime[14])*speed;
                    normalizedPos = normalize(PingPos[14]);                                  
                    break;
                case 15:
                    time = (CurrTime - PingTime[15])*speed;
                    normalizedPos = normalize(PingPos[15]);                                  
                    break;
                case 16:
                    time = (CurrTime - PingTime[16])*speed;
                    normalizedPos = normalize(PingPos[16]);                                  
                    break;
                case 17:
                    time = (CurrTime - PingTime[17])*speed;
                    normalizedPos = normalize(PingPos[17]);                                  
                    break;
                case 18:
                    time = (CurrTime - PingTime[18])*speed;
                    normalizedPos = normalize(PingPos[18]);                                  
                    break;
                default:
                    pingedCheck = false;
                    break;
            }
            
            if(pingedCheck) {
                normalizedWorldPos = normalize(world_Position);
                dist = length(normalizedWorldPos - normalizedPos);

                if ( (dist > 0.03*time) && (dist < 0.04*time) && (dist < max_radius) ) {
                      color = vec4(1.0, 1.0, 1.0, 1.0);
                } 
            }           
            
             
        }
        
    }

