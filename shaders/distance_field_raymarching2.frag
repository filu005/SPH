#version 330 core
uniform sampler3D density_texture;
uniform sampler2D front_volume_texture;
uniform sampler2D back_volume_texture;

in vec4 projected_pos_coords;

out vec4 color;
 
void main()
{
	// diagonal of the cube
	const float maxDist = sqrt(3.0);
	const int numSamples = 256;
	const float scale = maxDist/float(numSamples);
 
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
 
	vec4 dst = vec4(0, 0, 0, 0);
	vec4 src = vec4(0, 0, 0, 0);
 
	float value = 0.0f;

	for(int i = 0; i < numSamples; i++)
	{
		value = texture(density_texture, pos).r;
			 
		src = vec4(value);
		src.a *= .5f; //reduce the alpha to have a more transparent result 
		 
		//Front to back blending
		// dst.rgb = dst.rgb + (1 - dst.a) * src.a * src.rgb
		// dst.a   = dst.a   + (1 - dst.a) * src.a     
		src.rgb *= src.a;
		dst = (1.0f - dst.a)*src + dst;     
	 
		//break from the loop when alpha gets high enough
		if(dst.a >= .95f)
			break; 
	 
		//advance the current position
		pos += step_dir;
	 
		//break if the position is greater than <1, 1, 1>
		if(pos.x > 1.0f && pos.y > 1.0f && pos.z > 1.0f)
			break;
	}
 
	color = dst;
}