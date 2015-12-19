#version 330 core
layout (location = 0) in vec3 position;
// layout (location = 1) in vec3 tex_coords;

// out vec3 tex_coord;
out vec4 posout;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;


void main()
{
	const float one_volBoundingBox_ratio = 2.0f;
	
    vec4 pos = projection * view * model * vec4(position, 1.0);
    posout = vec4(position * one_volBoundingBox_ratio, 1.0);
    // tex_coord = tex_coords;
    gl_Position = pos;
}