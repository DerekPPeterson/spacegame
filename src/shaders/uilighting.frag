#version 330 core
in vec2 TexCoord;
in vec3 normal;
in vec3 fragPos;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

uniform bool useTextures = true;
uniform sampler2D diffuse0;
uniform sampler2D specular0;
uniform sampler2D emissive0;

// used in addition to the texture color
// set to 0 if using just textures
uniform vec3 diffuseColor = vec3(0, 0, 0);
uniform vec3 specularColor = vec3(0, 0, 0);

uniform float gamma = 1.0;

struct DirLight {
    vec3 lightDir;
    vec3 color;
};

uniform float ambientStrength = 0.3;
uniform float specularStrength = 2.0;


vec3 calcDirLight(DirLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = light.lightDir;
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 diffuse;
    if (useTextures) {
        diffuse = diff * light.color * (texture(diffuse0, TexCoord).rgb + diffuseColor);
    } else {
        diffuse = diff * light.color * (diffuseColor);
    }

    vec3 reflectDir = reflect(-lightDir, normal);

    vec3 specular;
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    if (useTextures) {
        specular = specularStrength * spec * light.color * (texture(specular0, TexCoord).rgb + specularColor);
    } else {
        specular = specularStrength * spec * light.color * (specularColor);
    }

    return (diffuse + specular);
}

void main()
{
    vec3 totalLight; // Used to accumulate current light value from all sources

    // Ambient
    totalLight = ambientStrength * texture(diffuse0, TexCoord).rgb;

    DirLight light;
    light.lightDir = normalize(vec3(0.5, 0.5, -1));
    light.color = vec3(1, 1, 1);

    vec3 viewDir = vec3(0, 0, -1);
    totalLight += calcDirLight(light, normal, fragPos, viewDir);

    // reinhard tone mapping
    vec3 mapped = totalLight / (totalLight + vec3(1.0));
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
    FragColor = vec4(mapped, 1);

    float brightness = dot(totalLight.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(totalLight.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
