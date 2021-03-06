#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 offset;

out vec3 outColor;

uniform float bin_count;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position + offset, 1.0);
    outColor = vec3(1.0, 0.75, 0.75) - vec3(0.2, 1.0, 1.0) * (gl_InstanceID / bin_count);
}