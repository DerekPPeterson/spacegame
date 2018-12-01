#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoords;

uniform vec3 color = vec3(0.5, 0, 0);
uniform sampler2D fontTexture;

void main()
{
    //FragColor = vec4(0,1,0, 1);
    vec4 distanceFieldColor = texture(fontTexture, TexCoords);
    if (distanceFieldColor.r < 0.5) {
        discard;
    }
    FragColor = vec4(color, 1) ;

    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 0);
}
