#version 330 core
in vec4 vertexColor;
in vec2 TexCoord;
in vec3 normal;
in vec3 fragPos;

out vec4 FragColor;

uniform sampler2D diffuse0;
uniform sampler2D specular0;

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

float ambientStrength = 0.3;
float specularStrength = 2.0;
vec3 color = vec3(0.7, 0.6, 0.1);

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

    FragColor = vec4(ambient, 1) + vec4(diffuse, 1) + vec4(specular, 1);
}
