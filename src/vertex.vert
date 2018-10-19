#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec4 vertexColor;
out vec2 TexCoord;

uniform float offset;
uniform mat4 transform;

vec4 tmp;

void main()
{
   gl_Position = transform * vec4(aPos, 1);
   //gl_Position = vec4(tmp.x + offset, tmp.yz, 1.0);
   //gl_Position = vec4(aPos, 1);
   vertexColor = vec4(aColor, 1);
   TexCoord = aTexCoord;
}
