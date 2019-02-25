#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;
  
in vec2 TexCoords;

uniform sampler2D tex;
uniform float gamma = 1.0;

void main()
{             
    //const float gamma = 1.0;
    //vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
  
    //// reinhard tone mapping
    //vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    //// gamma correction 
    //mapped = pow(mapped, vec3(1.0 / gamma));
  
    //FragColor = vec4(mapped, 1.0);
    vec3 color = texture(tex, TexCoords).rgb;

    vec3 mapped = color / (color + vec3(1.0));
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
    FragColor = vec4(mapped, 1);

    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(color.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}    
