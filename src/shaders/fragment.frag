#version 330 core
in vec4 vertexColor;
in vec2 TexCoord;
in vec3 normal;
in vec3 fragPos;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

uniform sampler2D diffuse0;
uniform sampler2D specular0;

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

float ambientStrength = 0.3;
float specularStrength = 2.0;
vec3 color = vec3(0.7, 0.6, 0.1);

uniform float gamma = 1.0;

void main()
{
    vec3 lightDir = normalize(lightPos - fragPos);
    vec3 normNorm  = normalize(normal);

    float diff = max(dot(normNorm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * texture(diffuse0, TexCoord).rgb;

    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normNorm);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor * texture(specular0, TexCoord).rgb;

    vec3 ambient = ambientStrength * texture(diffuse0, TexCoord).rgb;

    vec3 litColor = ambient + diffuse + specular;
    
    // reinhard tone mapping
    vec3 mapped = litColor / (litColor + vec3(1.0));
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
    FragColor = vec4(mapped, 1);

    float brightness = dot(litColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(litColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
