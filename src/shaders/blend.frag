#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D hdrBuffer0; // main
uniform sampler2D hdrBuffer1; // bloom
uniform sampler2D hdrBuffer2; // warp

void main()
{             
    const float gamma = 1.0;
    vec3 hdrColor;
    vec3 hdrColor0 = texture(hdrBuffer0, TexCoords).rgb;
    vec3 hdrColor1 = texture(hdrBuffer1, TexCoords).rgb;
    vec3 hdrColor2 = texture(hdrBuffer2, TexCoords).rgb;

    // If the warp texture is filled in then us it instead of the main texture
    if (hdrColor2.x + hdrColor2.y + hdrColor2.z > 0) {
        hdrColor = hdrColor2 + hdrColor1;
    } else {
        hdrColor = hdrColor0 + hdrColor1;
    }
  
    // reinhard tone mapping
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    FragColor = vec4(mapped, 1.0);
}    
