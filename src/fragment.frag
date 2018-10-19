#version 330 core
in vec4 vertexColor;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D tex1;
uniform sampler2D tex2;

void main()
{
   FragColor = texture(tex2, TexCoord) ;
}
