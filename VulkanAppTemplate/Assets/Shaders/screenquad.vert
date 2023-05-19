#version 460

layout (location = 0) out vec2 texcoord;

void main()
{
    // gl_Position = vec4(inPosition, 1.0);
    // texcoord = inTexcoord;
    texcoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(texcoord * 2.0 + -1.0, 0.0, 1.0);
}