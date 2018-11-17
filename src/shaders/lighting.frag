#version 330 core
in vec2 TexCoord;
in vec3 normal;
in vec3 fragPos;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

uniform sampler2D diffuse0;
uniform sampler2D specular0;
uniform sampler2D emissive0;

uniform vec3 viewPos;

struct PointLight {
    vec3 position;
    vec3 color;
    vec3 attenuation; // (constant, linear, quadratic)
};

#define MAX_POINT_LIGHTS 50
uniform int numPointLights;
uniform PointLight pointLights[MAX_POINT_LIGHTS];

uniform float ambientStrength = 0.3;
uniform float specularStrength = 2.0;


vec3 CalcPointLight(PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDir) 
{
    vec3 lightDir = normalize(pointLight.position - fragPos);
    normal = normalize(normal);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * pointLight.color * texture(diffuse0, TexCoord).rgb;

    vec3 reflectDir = reflect(-lightDir, normal);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * pointLight.color * texture(specular0, TexCoord).rgb;

    float dist = distance(fragPos, pointLight.position);
    float attenuation = 1.0f / (pointLight.attenuation[0] 
            + pointLight.attenuation[1] * dist 
            + pointLight.attenuation[2] * dist * dist);

    return attenuation * (diffuse + specular);
}

void main()
{
    vec3 totalLight; // Used to accumulate current light value from all sources

    // Ambient
    totalLight = ambientStrength * texture(diffuse0, TexCoord).rgb;

    // PointLights
    vec3 viewDir = normalize(viewPos - fragPos);
    for (int i = 0; i < numPointLights; i++) {
        totalLight += CalcPointLight(pointLights[i], normal, fragPos, viewDir);
    }

    FragColor = vec4(totalLight, 1);

    // If bright output to brightColor texture for bloom
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
