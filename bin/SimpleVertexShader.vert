    #version 330 core
     
    // Input vertex data, different for all executions of this shader.
    layout(location = 0) in vec2 texCoords;
    layout(location = 1) in vec3 normals;
    layout(location = 2) in vec3 vertPositions;
     
    uniform mat4 MVP;
     
    out vec2 UV;
     
    void main()
    {
    	// Output position of the vertex, in clip space : MVP * position
    	gl_Position =  MVP * vec4(vertPositions, 1.0);
    	UV = texCoords;
    }

