#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 warpCentreScreen;
in vec2 TexCoords;
  
uniform sampler2D hdrBuffer0;
uniform sampler2D hdrBuffer1;
uniform sampler2D normalAdjustBuffer;

uniform vec2 screenSize;

uniform vec2 centre = vec2(0.5, 0.5);

void main()
{             
    // coords of buffer with image drawn in
    vec2 bufTexCoords = vec2(gl_FragCoord.x / screenSize.x, gl_FragCoord.y / screenSize.y);

    vec3 normal = normalize(vec3(texture(normalAdjustBuffer, bufTexCoords).rg, 1));

    bufTexCoords += refract(vec3(0, 0, 1), normal, 2).xy * 0.1;
    FragColor = texture(hdrBuffer0, bufTexCoords);
    //FragColor = vec4(texture(normalAdjustBuffer, bufTexCoords).rg, 1, 1);

    BrightColor = texture(hdrBuffer1, bufTexCoords);
}    
