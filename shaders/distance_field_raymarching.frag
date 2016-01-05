#version 330 core
uniform sampler3D density_texture;
uniform sampler2D front_volume_texture;
uniform sampler2D back_volume_texture;
uniform samplerCube skybox;
uniform vec3 camera_position;
uniform mat4 model;

in vec4 projected_pos_coords;

out vec4 color;

void main()
{
	vec3 g_lightPos = vec3(3.0, 0.5, 0.5);
	vec3 g_lightIntensity = vec3(1.3, 1.2, 3.3);
	float g_absorption = 2.25;
	// diagonal of the cube
	const float maxDist = sqrt(3.0);

	// increases the number of steps for a single ray
	const float sampling_quality = 1.0f/8.0f;

	const int numSamples = 64 * int(1.0f/sampling_quality);
	const float scale = maxDist/float(numSamples);

	const int numLightSamples = 256;
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

	vec3 surface_pos = vec3(0.0f, 0.0f, 0.0f);

	for (int i=0; i < numSamples; ++i)
	{
		// If you want a texture access to return the exact value stored
		// at a voxel despite using GL_LINEAR you can make sure that your
		// texture coordinates fall at the center of a voxel. For example
		// to sample the value at voxel location (ix, iy, iz) from a texture
		// of size (sx, sy, sz) use texture coordinates
		// ((ix+0.5)/sx), (iy+0.5)/sy, (iz+0.5)/sz)), where ix,iy,iz is
		// the zero based integer location of the voxel.


		// sample density
		// sampling step size from distance field
		float density = texture(density_texture, pos).a;
		step_dir = dir*density;

		// skip empty space
		if (density <= 0.025f) // this const should be uniform value, slightly bigger than c::rmin, 0.015f
		{
			//if(surface_pos.r == 0.0f && surface_pos.g == 0.0f && surface_pos.b == 0.0f)
			if(all(equal( surface_pos, vec3(0.0f) )))
				surface_pos = pos;
			// attenuate ray-throughput
			T *= 1.0-density*g_absorption;//*scale
			if (T <= 0.01)
				break;

			// point light step_dir in texture space
			vec3 lightDir = normalize(g_lightPos-pos);//*lscale;

			// sample light
			float Tl = 1.0; // transmittance along light ray
			vec3 lpos = pos + lightDir;

			for (int s=0; s < numLightSamples; ++s)
			{
				float ld = texture(density_texture, lpos).a;
				Tl *= 1.0-g_absorption*ld;//*lscale

				if (Tl <= 0.01)
					break;

				lpos += lightDir;
			}

			vec3 Li = g_lightIntensity*Tl;

			Lo += T*density;//*scale;Li*
		}

		// advance the current position
		pos += step_dir * sampling_quality;
	}

	// cube mapping
	// normal computed from voxel distance field by central difference
	vec3 normal = texture(density_texture, surface_pos).rgb;
	vec3 Normal = mat3(transpose(inverse(model))) * normal;
	vec3 Position = vec3(model * vec4(surface_pos, 1.0f));

	float refraction_ratio = 1.00 / 1.33;
	vec3 I = normalize(Position - camera_position);
	vec3 R = refract(I, normalize(Normal), refraction_ratio);
	color.rgb = texture(skybox, R).rgb;

	//color.rgb = Lo;
	color.a = 1.0-T;
}