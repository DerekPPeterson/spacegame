#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 warpCentreScreen;
in vec2 TexCoords;
  
uniform sampler2D hdrBuffer;
uniform sampler2D normalAdjustBuffer;

uniform vec2 screenSize;

uniform vec2 centre = vec2(0.5, 0.5);

void main()
{             
    // coords of buffer with image drawn in
    vec2 bufTexCoords = vec2(gl_FragCoord.x / screenSize.x, gl_FragCoord.y / screenSize.y);

    vec3 normal = normalize(vec3(texture(normalAdjustBuffer, bufTexCoords).rg, 1));

    bufTexCoords += refract(vec3(0, 0, 1), normal, 2).xy * 0.1;
    FragColor = texture(hdrBuffer, bufTexCoords);
    
    // If bright output to brightColor texture for bloom
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}    
