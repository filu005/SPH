#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in float bin_idx;
layout (location = 2) in mat4 model_mat;
layout (location = 6) in int at_surface;
layout (location = 7) in int particle_color_mod;
layout (location = 8) in int transparency;

out vec4 outColor;

uniform float bin_count;

uniform mat4 view;
uniform mat4 projection;

void main()
{
	// gl_PointSize = 7.0;
	gl_Position = projection * view * model_mat * vec4(position, 1.0);
	float scaled_pressure = mix(0.0, 1.0, particle_color_mod);
	
	if (particle_color_mod == 0){  // healthy
		outColor = 0.5*vec4(scaled_pressure, 0.0, 1.0 - scaled_pressure, transparency*0.0) + 0.5*vec4(1.0, 0.0, 1.0,  transparency*0.0);
	}  //tumor in proliferative  0  255  0
	else if(particle_color_mod == 1) {
		outColor = vec4(0.0, 1.0, 0.0, transparency);
	}	//tumor in quiescent   233  233  12
	else if(particle_color_mod == 2) {
		outColor = vec4(0.91, 0.91, 0.04, transparency);
	}	//tumor in dead   12  7  152
	else if(particle_color_mod == 3) {
		outColor = vec4(0.04, 0.02, 0.59, transparency);
	}   // blood vessels
	else if(particle_color_mod == 4){
		outColor = 0.5*vec4(scaled_pressure, 0.0, 1.0 - scaled_pressure, transparency) + 0.5*vec4(1.0, 0.0, 1.0, transparency);
	} 

} 