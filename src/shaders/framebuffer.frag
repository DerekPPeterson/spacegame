#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D hdrBuffer;

void main()
{             
    //const float gamma = 1.0;
    //vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
  
    //// reinhard tone mapping
    //vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    //// gamma correction 
    //mapped = pow(mapped, vec3(1.0 / gamma));
  
    //FragColor = vec4(mapped, 1.0);
    FragColor = texture(hdrBuffer, TexCoords);
}    
