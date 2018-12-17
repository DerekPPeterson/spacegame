#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 warpCentreScreen;
in vec2 TexCoords;
  
uniform sampler2D hdrBuffer0;
uniform sampler2D hdrBuffer1;

uniform vec2 screenSize;

uniform vec2 centre = vec2(0.5, 0.5);

void main()
{             
    // coords of buffer with image drawn in
    vec2 bufTexCoords = vec2(gl_FragCoord.x / screenSize.x, gl_FragCoord.y / screenSize.y);
    vec2 fromCentre = TexCoords - centre;

    float intensity = smoothstep(0.5, 0.40, length(fromCentre));
    vec4 texColor = texture(hdrBuffer0, bufTexCoords);
    vec4 invertedColor = vec4(vec3(1, 1, 1) - texColor.rgb, 1);

    FragColor = intensity * invertedColor + (1 - intensity) * texColor;

    BrightColor = -texture(hdrBuffer1, bufTexCoords).rgba * 2;
}    
