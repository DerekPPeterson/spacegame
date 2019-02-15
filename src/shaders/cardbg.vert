#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out vec2 TexCoords;

uniform bool useView = true;

void main() 
{
    gl_Position = projection * (useView ? view : mat4(1.0f)) * model * vec4(aPos, 1);
    TexCoords = aTexCoord;
}
