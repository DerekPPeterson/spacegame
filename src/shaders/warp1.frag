#version 330 core
out vec4 normalAdjust;

in vec2 TexCoords;
  
uniform vec2 screenSize;

uniform vec2 centre = vec2(0.5, 0.5);

void main()
{             
    // coords of buffer with image drawn in
    vec2 bufTexCoords = vec2(gl_FragCoord.x / screenSize.x, gl_FragCoord.y / screenSize.y);

    vec2 fromCentre = TexCoords - centre;

    float intensity = (-cos(min(length(fromCentre), 0.5) * 3.1415 * 4) + 1) / 2;
    normalAdjust = vec4(fromCentre * intensity, 0, 1);
}    
