#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform bool useView = true;

void main() 
{
    gl_Position = projection * (useView ? view : mat4(1.0f)) * model * vec4(aPos, 1);
}
