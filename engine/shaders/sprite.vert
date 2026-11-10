#version 450

layout(push_constant) uniform SpritePush
{
    vec2 position;
    vec2 size;
    vec2 viewport;
} push;

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_uv;

layout(location = 0) out vec2 out_uv;

void main()
{
    vec2 pixel = push.position + in_position * push.size;
    vec2 ndc = (pixel / push.viewport) * 2.0 - 1.0;
    gl_Position = vec4(ndc, 0.0, 1.0);
    out_uv = in_uv;
}
