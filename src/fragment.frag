#version 330 core
in vec4 vertexColor;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D tex1;
uniform sampler2D tex2;

uniform vec3 lightColor;

void main()
{
    vec4 tmp;
    if (texture(tex2, TexCoord) == vec4(0, 0, 0, 1)) {
        tmp = texture(tex1, TexCoord);
    } else {
        tmp = texture(tex2, TexCoord);
    }
    FragColor = tmp * vec4(lightColor, 1);
}
