#version 330 core
layout (location = 0) in vec3 position;

out vec4 projected_pos_coords;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;


void main()
{
    vec4 pos = projection * view * model * vec4(position, 1.0);
    projected_pos_coords = pos;
    gl_Position = pos;
}