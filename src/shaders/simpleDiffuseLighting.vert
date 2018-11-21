#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 diffuseColor;

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

vec3 CalcPointLight(PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDir) 
{
    vec3 lightDir = normalize(pointLight.position - fragPos);
    normal = normalize(normal);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 diffuse;
    diffuse = diff * pointLight.color * (diffuseColor);

    float dist = distance(fragPos, pointLight.position);
    float attenuation = 1.0f / (pointLight.attenuation[0] 
            + pointLight.attenuation[1] * dist 
            + pointLight.attenuation[2] * dist * dist);

    return attenuation * diffuse;
}


void main()
{
    vec3 totalLight; // Used to accumulate current light value from all sources

    // Ambient
    totalLight = ambientStrength * diffuseColor;
    vec3 normal = mat3(transpose(inverse(model))) * aNormal; 

    // PointLights
    vec3 viewDir = normalize(viewPos - aPos);
    for (int i = 0; i < numPointLights; i++) {
        totalLight += CalcPointLight(pointLights[i], normal, aPos, viewDir);
    }

   color = totalLight;

   gl_Position = projection * view * model * vec4(aPos, 1);
}
