#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 3) in vec4 model0;
layout (location = 4) in vec4 model1;
layout (location = 5) in vec4 model2;
layout (location = 6) in vec4 model3;
layout (location = 7) in vec3 aColor;
layout (location = 8) in int useView;
layout (location = 9) in vec3 particlePos;

out vec2 TexCoords;
out vec3 color;

uniform mat4 view;
uniform mat4 projection;

void main() 
{
    mat4 model = mat4(model0, model1, model2, model3);
    gl_Position = projection * (useView != 0 ? view : mat4(1.0f)) * model * vec4(particlePos, 1);
    color = aColor;
}

