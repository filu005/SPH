#include "Mathematics/GteIntpTricubic3.h"
#include "Painter.hpp"
#include "constants.hpp"
#include "DistanceField.hpp"


DistanceField::DistanceField() : voxel_field_size{64}
{
	perlin_noise_gen = noise_factory.create3D();
	setup_buffers();
}

DistanceField::~DistanceField()
{
	glDeleteTextures(1, &volume_texture);
	glDeleteRenderbuffers(1, &depth_volume_RBO);
	glDeleteTextures(1, &front_volume_texture);
	glDeleteTextures(1, &back_volume_texture);
	glDeleteFramebuffers(1, &front_back_volume_FBO);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}

void DistanceField::paint(Painter& p) const
{
	p.paint(*this);
}

void DistanceField::setup_buffers(void)
{
	// Create buffers/arrays
	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(this->VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(this->box_data), &box_data, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*) 0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*) (3 * sizeof(GLfloat)));

	// create distance field in 3D texture
	volume_texture = generate_voxel_texture3d();

	glBindVertexArray(0);

	// Create Framebuffer for two textures used in volume rendering-raycasting shader
	glGenFramebuffers(1, &front_back_volume_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, front_back_volume_FBO);

	// create a renderbuffer object to store depth info
	glGenRenderbuffers(1, &depth_volume_RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_volume_RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, c::width, c::height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// attach the renderbuffer to depth attachment point:
	glFramebufferRenderbuffer(GL_FRAMEBUFFER,      // 1. fbo target: GL_FRAMEBUFFER
		GL_DEPTH_ATTACHMENT, // 2. attachment point
		GL_RENDERBUFFER,     // 3. rbo target: GL_RENDERBUFFER
		depth_volume_RBO);   // 4. rbo ID

	// Create a color attachment texture:
	std::tie(front_volume_texture, back_volume_texture) = generate_front_back_color_cube_textures();
	// attach the texture to FBO color attachment point:
	//glFramebufferTexture2D(GL_FRAMEBUFFER,        // 1. fbo target: GL_FRAMEBUFFER 
	//	GL_COLOR_ATTACHMENT0,  // 2. attachment point
	//	GL_TEXTURE_2D,         // 3. tex target: GL_TEXTURE_2D
	//	textureId,             // 4. tex ID
	//	0);                    // 5. mipmap level: 0(base)
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, front_volume_texture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, back_volume_texture, 0);
	

	// Now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int DistanceField::get_voxel_index(glm::vec3 const v) const
{
	using namespace c;
	return static_cast<int>(
		floor((v.x - xyzminV) / voxelSize) +
		floor((v.y - xyzminV) / voxelSize) * static_cast<float>(voxelGridDimension) +
		floor((v.z - xyzminV) / voxelSize) * static_cast<float>(voxelGridDimension) * static_cast<float>(voxelGridDimension)
		);
}

bool DistanceField::out_of_voxel_grid_scope(glm::vec3 const idx) const
{
	using namespace c;
	return idx.x >= c::voxelGridDimension || idx.x < 0 || idx.y >= c::voxelGridDimension || idx.y < 0 || idx.z >= c::voxelGridDimension || idx.z < 0;
}

void DistanceField::generate_field_from_surface_particles(std::vector<Particle> const & surface_particles)
{
	// will contain a distance from vertex to the closest surface particle
	std::array<GLfloat, c::voxelGrid3dSize> voxel_grid;
	voxel_grid.fill(c::rmax);

	// for every surface particle
	for(auto const & p : surface_particles)
	{
		auto const p_pos = p.position;
		// bound it with 'bounding cube'

		auto const bounding_cube_center_shift = std::floor(c::boundingCubeScale * 0.5f);
		for(float i = -bounding_cube_center_shift; i <= bounding_cube_center_shift; ++i)
		{
			for(float j = -bounding_cube_center_shift; j <= bounding_cube_center_shift; ++j)
			{
				for(float k = -bounding_cube_center_shift; k <= bounding_cube_center_shift; ++k)
				{
					glm::vec3 voxel_pos{ p_pos.x + i * c::voxelSize, p_pos.y + j * c::voxelSize, p_pos.z + k * c::voxelSize };
					auto p_voxel_dist = glm::length(p_pos - voxel_pos);

					auto voxel_index = get_voxel_index(voxel_pos);
					// it is possible to out of grid's scope
					if(voxel_index >= c::voxelGrid3dSize)
						continue;

					auto current_dist = voxel_grid[voxel_index];

					if(p_voxel_dist >= current_dist)
						continue;
					if(p_voxel_dist < c::rmin)
						p_voxel_dist = c::rmin;
					
					voxel_grid[voxel_index] = p_voxel_dist;
					
				}
			}
		}
		// and next particle
	}

	auto const voxel_grid_normals = generate_normals_for_voxel_field(voxel_grid);

	// OpenGL part - transfer distance field to graphic card
	std::array<GLfloat, c::voxelGrid3dSize * 4> rgba_data;

	for(int i = 0; i < c::voxelGrid3dSize; ++i)
	{
		auto const normal = voxel_grid_normals[i];
		rgba_data[i * 4] = normal.x;
		rgba_data[i * 4 + 1] = normal.y;
		rgba_data[i * 4 + 2] = normal.z;
		rgba_data[i * 4 + 3] = voxel_grid[i];
	}
	

	// upload to DistanceField::volume_texture
	glBindTexture(GL_TEXTURE_3D, this->volume_texture);
	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, c::voxelGridDimension, c::voxelGridDimension, c::voxelGridDimension, GL_RGBA, GL_FLOAT, &rgba_data[0]);
	glBindTexture(GL_TEXTURE_3D, 0);
}

std::array<glm::vec3, c::voxelGrid3dSize> DistanceField::generate_normals_for_voxel_field(std::array<GLfloat, c::voxelGrid3dSize> const & voxel_grid)
{
	auto const sample_size = 1;
	std::array<glm::vec3, c::voxelGrid3dSize> voxel_grid_normals;

	for(int z = 0; z < c::voxelGridDimension; ++z)
	{
		for(int y = 0; y < c::voxelGridDimension; ++y)
		{
			for(int x = 0; x < c::voxelGridDimension; ++x)
			{
				glm::vec3 v1(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
				glm::vec3 v2 = v1;
				auto vidx = [](int x, int y, int z) -> int { return x + y * c::voxelGridDimension + z * c::voxelGridDimension * c::voxelGridDimension; };

				if(x >= sample_size && y >= sample_size && z >= sample_size &&
					x < c::voxelGridDimension - sample_size && y < c::voxelGridDimension - sample_size && z < c::voxelGridDimension - sample_size)
				{
					v1.x = voxel_grid[vidx(x - sample_size, y, z)];
					v2.x = voxel_grid[vidx(x + sample_size, y, z)];
					v1.y = voxel_grid[vidx(x, y - sample_size, z)];
					v2.y = voxel_grid[vidx(x, y + sample_size, z)];
					v1.z = voxel_grid[vidx(x, y, z - sample_size)];
					v2.z = voxel_grid[vidx(x, y, z + sample_size)];
					
				}
				glm::vec3 normal = glm::normalize(v2 - v1);
				if(std::isnan(normal.x))
					normal = glm::vec3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
				
				voxel_grid_normals[vidx(x, y, z)] = normal;
			}
		}
	}

	voxel_grid_normals = filter_voxel_field_normals(voxel_grid_normals);

	return voxel_grid_normals;
}


std::array<glm::vec3, c::voxelGrid3dSize> DistanceField::filter_voxel_field_normals(std::array<glm::vec3, c::voxelGrid3dSize> & voxel_grid_normals)
{
	auto const sample = 3;
	auto vidx = [](int x, int y, int z) -> int { return x + y * c::voxelGridDimension + z * c::voxelGridDimension * c::voxelGridDimension; };
	std::array<glm::vec3, c::voxelGrid3dSize> filtered_normals;

	for(int z = 0; z < c::voxelGridDimension; ++z)
	{
		for(int y = 0; y < c::voxelGridDimension; ++y)
		{
			for(int x = 0; x < c::voxelGridDimension; ++x)
			{
				auto const n = (sample - 1) / 2;
				glm::vec3 average_vec(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
				auto count = 0;

				for(int k = z - n; k <= z + n; ++k)
				{
					for(int j = y - n; j <= y + n; ++j)
					{
						for(int i = x - n; i <= x + n; ++i)
						{
							if(out_of_voxel_grid_scope(glm::vec3(i, j, k)))
								continue;

							average_vec += voxel_grid_normals[vidx(i, j, k)];
							++count;
						}
					}
				}
				average_vec /= static_cast<float>(count);
				glm::vec3 gradient = glm::normalize(average_vec);
				if(std::isnan(gradient.x))
					gradient = glm::vec3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());

				filtered_normals[vidx(x, y, z)] = gradient;
			}
		}
	}

	return filtered_normals;
}

GLuint DistanceField::generate_voxel_texture3d()
{
	GLuint texid;
	glGenTextures(1, &texid);

	GLenum target = GL_TEXTURE_3D;
	GLenum filter = GL_LINEAR;
	GLenum address = GL_CLAMP_TO_BORDER;

	glBindTexture(target, texid);

	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, filter);

	glTexParameteri(target, GL_TEXTURE_WRAP_S, address);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, address);
	glTexParameteri(target, GL_TEXTURE_WRAP_R, address);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	glTexImage3D(target,
		0,
		GL_RGBA,
		c::voxelGridDimension,
		c::voxelGridDimension,
		c::voxelGridDimension,
		0,
		GL_RGBA,
		GL_FLOAT,
		NULL);

	glBindTexture(target, 0);

	return texid;
}

GLuint DistanceField::generate_field()
{
	GLuint texid;
	glGenTextures(1, &texid);

	GLenum target = GL_TEXTURE_3D;
	GLenum filter = GL_LINEAR;
	GLenum address = GL_CLAMP_TO_BORDER;

	glBindTexture(target, texid);

	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, filter);

	glTexParameteri(target, GL_TEXTURE_WRAP_S, address);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, address);
	glTexParameteri(target, GL_TEXTURE_WRAP_R, address);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	GLubyte *data = new GLubyte[voxel_field_size*voxel_field_size*voxel_field_size];
	auto rgba_data = make_unique<GLubyte[]>(voxel_field_size*voxel_field_size*voxel_field_size * 4);
	GLubyte *ptr = &data[0];

	float frequency = 4.0f / voxel_field_size;
	float center = voxel_field_size / 2.0f + 0.5f;
	auto const r = 0.35f;
	auto const A = 0.25f;

	for(int x = 0; x < voxel_field_size; x++)
	{
		for(int y = 0; y < voxel_field_size; ++y)
		{
			for(int z = 0; z < voxel_field_size; ++z)
			{
				float dx = center - x;
				float dy = center - y;
				float dz = center - z;

				float off = fabsf(perlin_noise_gen->generate(x*frequency, y*frequency, z*frequency));
					
				//Perlin3D(x*frequency,y*frequency,z*frequency,5,0.5f);

				float d = sqrtf(dx*dx + dy*dy + dz*dz) / (voxel_field_size);

				*ptr++ = ((d - off) < r) ? 255 : 0;
				//*ptr++ = (A*(d-r)*off)*255;
			}
		}
	}

	for(int i = 0; i < voxel_field_size*voxel_field_size*voxel_field_size; ++i)
	{
		rgba_data[i * 4] = data[i];
		rgba_data[i * 4 + 1] = data[i];
		rgba_data[i * 4 + 2] = data[i];
		rgba_data[i * 4 + 3] = data[i];
	}

	// upload
	glTexImage3D(target,
		0,
		GL_RGBA,
		voxel_field_size,
		voxel_field_size,
		voxel_field_size,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		rgba_data.get());
	
	glBindTexture(target, 0);

	delete[] data;

	return texid;
}

std::pair<GLuint, GLuint> const DistanceField::generate_front_back_color_cube_textures()
{
	std::array<GLuint, 2> tex_front_back;

	glGenTextures(2, &tex_front_back[0]);

	for(auto const tex : tex_front_back)
	{
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, c::width, c::height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	return std::make_pair(tex_front_back[0], tex_front_back[1]);
}

/*
Remember: to specify vertices in a counter-clockwise winding order you need to visualize the triangle
as if you're in front of the triangle and from that point of view, is where you set their order.

To define the order of a triangle on the right side of the cube for example, you'd imagine yourself looking
straight at the right side of the cube, and then visualize the triangle and make sure their order is specified
in a counter-clockwise order. This takes some practice, but try visualizing this yourself and see that this
is correct.
*/

GLfloat const DistanceField::box_data[] =
{
	// Back face
	-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
	1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f,
	1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
	1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f,
	-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
	-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	// Front face
	-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
	-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	// Left face
	-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
	-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
	-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
	// Right face
	1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
	1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f,
	1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
	1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
	// Bottom face
	-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
	1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
	1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
	1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
	-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
	// Top face
	-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f,
	1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f
};
