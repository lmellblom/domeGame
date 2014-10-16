    #version 330 core
     
    uniform sampler2D Tex;
     
    in vec2 uv;
    out vec4 color;
     
    void main()
    {
    	color = texture(Tex, uv.st);
    }

