#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in float depth;

uniform bool resetDepth = false;

void main()
{
    //discard;
    FragColor = vec4(1, 0, 0, 1);

    //if (resetDepth) {
    //    gl_FragDepth = 1;
    //} else {
    //    gl_FragDepth = depth;
    //}
}
