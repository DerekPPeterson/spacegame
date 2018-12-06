#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoords;

uniform vec3 color = vec3(0.5, 0, 0);
uniform sampler2D fontTexture;

uniform float edgestart = 0.45;
uniform float edgeend = 0.55;

uniform float gamma = 1.0;

void main()
{
    //FragColor = vec4(0,1,0, 1);
    vec4 distanceFieldColor = texture(fontTexture, TexCoords);
    vec4 outColor = vec4(color, 1) * smoothstep(edgestart, edgeend, texture(fontTexture, TexCoords).r);

    vec3 mapped = outColor.rgb / (outColor.rgb + vec3(1.0));
    
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
    FragColor = vec4(mapped, outColor.a);

    float brightness = dot(outColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(outColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
