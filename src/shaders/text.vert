#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec2 TexCoordOffset = vec2(0, 0);
uniform vec2 TexCoordSize = vec2(1, 1);
uniform vec2 charSize = vec2(1, 1);

void main() 
{
    TexCoords.x = charSize.x * aTexCoord.x + TexCoordOffset.x;
    TexCoords.y = charSize.y * (aTexCoord.y) + TexCoordOffset.y;
    gl_Position = projection * model * vec4(aPos, 1);
    //gl_Position = vec4(aPos, 1);
}
