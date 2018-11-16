#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D hdrBuffer0;
uniform sampler2D hdrBuffer1;

void main()
{             
    const float gamma = 1.0;
    vec3 hdrColor0 = texture(hdrBuffer0, TexCoords).rgb;
    vec3 hdrColor1 = texture(hdrBuffer1, TexCoords).rgb;
    vec3 hdrColor = hdrColor0 + hdrColor1;
  
    // reinhard tone mapping
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    FragColor = vec4(mapped, 1.0);
}    
