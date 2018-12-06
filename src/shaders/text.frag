#version 330 core
layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

uniform vec3 color = vec3(0.5, 0, 0);
uniform sampler2D fontTexture;

uniform float edgestart = 0.45;
uniform float edgeend = 0.55;

void main()
{
    //FragColor = vec4(0,1,0, 1);
    vec4 distanceFieldColor = texture(fontTexture, TexCoords);
    FragColor = vec4(color, 1) * smoothstep(edgestart, edgeend, texture(fontTexture, TexCoords).r);
}
