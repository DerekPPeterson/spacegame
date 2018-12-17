#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;
  
in vec2 TexCoords;

uniform sampler2D hdrBuffer;

void main()
{             
    // If bright output to brightColor texture for bloom
    vec4 texColor = texture(hdrBuffer, TexCoords);
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
        if(brightness > 1.0)
            FragColor = vec4(texColor.rgb, 1.0);
        else
            FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}    
