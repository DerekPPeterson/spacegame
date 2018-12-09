#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 3) in vec4 model0;
layout (location = 4) in vec4 model1;
layout (location = 5) in vec4 model2;
layout (location = 6) in vec4 model3;
layout (location = 7) in vec3 aColor;
layout (location = 8) in vec2 TexCoordOffset;
layout (location = 9) in vec2 charSize;

out vec2 TexCoords;
out vec3 color;

uniform mat4 view;
uniform mat4 projection;

void main() 
{
    mat4 model = mat4(model0, model1, model2, model3);
    TexCoords.x = charSize.x * aTexCoord.x + TexCoordOffset.x;
    TexCoords.y = charSize.y * (aTexCoord.y) + TexCoordOffset.y;
    gl_Position = projection * model * vec4(aPos, 1);
    //gl_Position = vec4(aPos, 1);
    color = aColor;
}
