#version 330 core
in vec4 vertexColor;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D tex1;
uniform sampler2D tex2;

void main()
{
   if (texture(tex2, TexCoord) == vec4(0, 0, 0, 1)) {
	   FragColor = texture(tex1, TexCoord) ;
   } else {
	   FragColor = texture(tex2, TexCoord);
   }
}
