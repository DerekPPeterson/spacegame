#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 3) in vec4 model0;
layout (location = 4) in vec4 model1;
layout (location = 5) in vec4 model2;
layout (location = 6) in vec4 model3;
layout (location = 7) in vec3 aColor;

out vec2 TexCoords;
out vec3 color;

uniform mat4 view;
uniform mat4 projection;

void main() 
{
    mat4 model = mat4(model0, model1, model2, model3);
    gl_Position = projection * view * model * vec4(aPos, 1);
    color = aColor;
}
