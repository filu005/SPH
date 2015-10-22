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
	volume_texture = generate_field();

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

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

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

	//for(int i = 0; i < voxel_field_size*voxel_field_size*voxel_field_size; ++i)
	//	std::cout << (int)data[i] << "\n";

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
