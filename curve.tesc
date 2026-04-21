#version 410 core
layout(vertices = 1) out;

uniform int numCP;

void main()
{
    gl_TessLevelOuter[0] = 128.0;
    gl_TessLevelInner[0] = 128.0;
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}