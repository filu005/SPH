#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in float bin_idx;
layout (location = 2) in mat4 model_mat;
layout (location = 6) in uint at_surface;

out vec3 outColor;

uniform float bin_count;

uniform mat4 view;
uniform mat4 projection;

void main()
{
	// gl_PointSize = 7.0;
    gl_Position = projection * view * model_mat * vec4(position, 1.0);
    if(at_surface > 0u)
    	outColor = vec3(0.0, 0.75, 0.75);
    else
    	outColor = vec3(1.0, 0.75, 0.75) - vec3(0.1, 0.9, 0.9) * (bin_idx / bin_count);
}