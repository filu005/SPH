#version 330 core
layout (location = 0) in vec2 position;
layout (location = 1) in vec2 offset;

out vec3 outColor;

uniform float bin_count;

void main()
{
    gl_Position = vec4(position + offset, 0.0, 1.0);
    outColor = vec3(1.0, 0.75, 0.75) - vec3(0.2, 1.0, 1.0) * (gl_InstanceID / bin_count);
}