#version 330 core
uniform sampler3D density_texture;
uniform sampler2D front_volume_texture;
uniform sampler2D back_volume_texture;

in vec4 projected_pos_coords;

out vec4 color;
 
void main()
{
	vec3 g_lightPos = vec3(2.0, 2.0, 2.5);
	vec3 g_lightIntensity = vec3(3.3, 3.2, 3.3);
	float g_absorption = 2.25;
	// diagonal of the cube
	const float maxDist = sqrt(3.0);
 
	const int numSamples = 128;
	const float scale = maxDist/float(numSamples);
 
	const int numLightSamples = 64;
	const float lscale = maxDist / float(numLightSamples);
 
	//calculate projective cube's texture coordinates
	//used to project the front and back position textures onto the cube
	vec2 projected_cube_coords = vec2(((projected_pos_coords.x / projected_pos_coords.w) + 1.0f ) / 2.0f,
						((projected_pos_coords.y / projected_pos_coords.w) + 1.0f ) / 2.0f );

	// sample front and back lookup textures
	// they look like this: http://www.voreen.org//files/ray_entry_exit.jpg
	vec3 front = texture(front_volume_texture, projected_cube_coords).xyz;
	vec3 back = texture(back_volume_texture, projected_cube_coords).xyz;

	//get entry position and ray's direction
	vec3 pos = front;
	vec3 dir = normalize(back - front);

	vec3 step_dir = dir*scale;
 
	// transmittance
	float T = 1.0;
	// in-scattered radiance
	vec3 Lo = vec3(0.0);
 
	for (int i=0; i < numSamples; ++i)
	{
		// sample density
		float density = texture(density_texture, pos).r;
 
		// skip empty space
		if (density > 0.0)
		{
			// attenuate ray-throughput
			T *= 1.0-density*scale*g_absorption;
			if (T <= 0.01)
				break;
 
			// point light step_dir in texture space
			vec3 lightDir = normalize(g_lightPos-pos)*lscale;
 
			// sample light
			float Tl = 1.0; // transmittance along light ray
			vec3 lpos = pos + lightDir;
 
			for (int s=0; s < numLightSamples; ++s)
			{
				float ld = texture(density_texture, lpos).r;
				Tl *= 1.0-g_absorption*lscale*ld;
 
				if (Tl <= 0.01)
					break;
 
				lpos += lightDir;
			}
 
			vec3 Li = g_lightIntensity*Tl;
 
			Lo += Li*T*density*scale;
		}
 
		pos += step_dir;
	}
 
	color.rgb = Lo;
	color.a = 1.0-T;
}