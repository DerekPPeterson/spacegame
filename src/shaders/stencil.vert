#version 330 core

layout (location = 0) in vec3 aPos;

out float depth;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform bool useView = false;

void main() 
{
    gl_Position = projection * (useView ? view : mat4(1.0f)) * model * vec4(aPos, 1);
    depth = gl_Position.z;
}
