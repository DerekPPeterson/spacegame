#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec4 vertexColor;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


vec4 tmp;

void main()
{
   gl_Position = projection * view * model * vec4(aPos, 1);
   //gl_Position = vec4(tmp.x + offset, tmp.yz, 1.0);
   //gl_Position = vec4(aPos, 1);
   TexCoord = aTexCoord;
   vertexColor = vec4(1, 0, 0, 1);
}
