#version 330 core
out vec2 normalAdjust;

in vec2 TexCoords;
  
uniform vec2 screenSize;

uniform vec2 centre = vec2(0.5, 0.5);

uniform float frequency = 0;
uniform float phase = 0;

void main()
{             
    // coords of buffer with image drawn in
    vec2 fromCentre = TexCoords - centre;

    float intensity = (-cos(min(length(fromCentre), 0.5) * 3.1415 * 4) + 1) / 2;
    float waves = cos(3.1415 * 2 * frequency * length(fromCentre) + phase);
    normalAdjust = vec2(fromCentre * intensity * waves);
}    
