#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in float bin_idx;
layout (location = 2) in vec3 offset;

out vec3 outColor;

uniform float bin_count;

void main()
{
	//gl_PointSize = 5.0;
    gl_Position = vec4(position + offset, 1.0);
    outColor = vec3(1.0, 0.75, 0.75) - vec3(0.1, 0.9, 0.9) * (bin_idx / bin_count);
}