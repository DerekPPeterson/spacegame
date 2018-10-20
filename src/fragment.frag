#version 330 core
in vec4 vertexColor;
in vec2 TexCoord;
in vec3 normal;
in vec3 fragPos;

out vec4 FragColor;

uniform sampler2D tex1;
uniform sampler2D tex2;

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
float ambientStrength = 0.1;
float specularStrength = 2.0;

void main()
{
    vec4 tmp;
    if (texture(tex2, TexCoord) == vec4(0, 0, 0, 1)) {
        tmp = texture(tex1, TexCoord);
    } else {
        tmp = texture(tex2, TexCoord);
    }
    vec3 lightDir = normalize(lightPos - fragPos);
    vec3 normNorm  = normalize(normal);

    float diff = max(dot(normNorm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * tmp.rgb;

    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normNorm);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor * texture(tex2, TexCoord).rgb; 

    FragColor = tmp * vec4(lightColor, 1) * ambientStrength + vec4(diffuse, 1) + vec4(specular, 1);
}
