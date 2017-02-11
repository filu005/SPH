#include <array>
#include <SOIL.h>

#include <stdlib.h> //realpath
#include <sys/types.h>
#include <sys/stat.h> //mkdir
#include <direct.h> //VS _mkdir

#include "Painter.hpp"
#include "ParticleSystem.hpp"
#include "Grid.hpp"
#include "MarchingCubes.h"
#include "constants.hpp"
#include "MCMesh.hpp"



#if defined(_WIN32) || defined(_WIN64)
// Copied from linux libc sys/stat.h:
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif

MCMesh::MCMesh()
{
	setup_buffers();
}

MCMesh::~MCMesh()
{
	//exportOBJ("mesh.obj");
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}

void MCMesh::paint(Painter& p) const
{
	p.paint(*this);
}

void MCMesh::setup_buffers(void)
{
	// Create buffers/arrays
	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &this->VBO);

	glBindVertexArray(this->VAO);
	// Load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	// tworze bufor o z gory ustalonym (maksymalnym) rozmiarze
	glBufferData(GL_ARRAY_BUFFER, vertices_buffer_size * sizeof(Vertex), &(this->vertices_buffer[0]), GL_DYNAMIC_DRAW);

	// Set the vertex attribute pointers
	// glVertexAttribPointer(location = 0, vec3 = 3, dataType = GL_FLOAT, normalise = GL_FALSE, strideSize = sizeof(struct), dataBegin = 0)
	// Vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, v));
	// Normal vector position
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, n));

	glBindVertexArray(0);

	loadTextures();
}

namespace kernel
{
	float W_poly6(float r_sq, float h_sq, float h)
	{
		static float coefficient = 315.0f / (64.0f*c::PIf*pow(h, 9));

		return coefficient * pow(h_sq - r_sq, 3);
	}
}

void MCMesh::generate_mesh(std::array<GridCell, c::C> const & grid)
{
	using particle_system::get_cell_index;
	using particle_system::out_of_grid_scope;
	using namespace c;

	auto const h_sq = c::H*c::H;
	auto const MCGridSize = (c::voxelGridDimension + 1) * (c::voxelGridDimension + 1) * (c::voxelGridDimension + 1);
	auto xyzw_data = make_unique<glm::vec4[]>(MCGridSize);

	for(int i = 0; i < c::voxelGridDimension + 1; i++)
	{
		for(int j = 0; j < c::voxelGridDimension + 1; j++)
		{
			for(int k = 0; k < c::voxelGridDimension + 1; k++)
			{
				auto idx = k + j*(c::voxelGridDimension + 1) + i*(c::voxelGridDimension + 1)*(c::voxelGridDimension + 1);
				auto cell_vertex_position = glm::vec3(c::xyzminV + static_cast<float>(k)*c::voxelSize, c::xyzminV + static_cast<float>(j) *c::voxelSize, c::xyzminV + static_cast<float>(i) *c::voxelSize);
				auto density = 0.0f;

				// get potential (density) from neighbouring 8 cells (voxels)
				// to create 3D scalar field on which Marching Cubes can run
				for(auto x = -0.5f; x < 1.0f; x = x + 1.0f) // x < 1.0f zeby na pewno zaliczyc -0.5 i 0.5
				{
					for(auto y = -0.5f; y < 1.0f; y = y + 1.0f)
					{
						for(auto z = -0.5f; z < 1.0f; z = z + 1.0f)
						{
							auto neighbour_cell_point = cell_vertex_position + glm::vec3(z*c::voxelSize, y*c::voxelSize, x*c::voxelSize);
							
							if(out_of_grid_scope(neighbour_cell_point))
								continue;

							int neighbour_grid_idx = get_cell_index(neighbour_cell_point);
							if(neighbour_grid_idx < 0 || neighbour_grid_idx >= c::C)
								continue;

							auto particle_ptr = grid[neighbour_grid_idx].first_particle;

							for(int particle_idx = 0; particle_idx < grid[neighbour_grid_idx].no_particles; ++particle_idx)
							{
								Particle& particle = *particle_ptr;
								
								glm::vec3 rVec = cell_vertex_position - particle.position;
								float r_sq = dot(rVec, rVec);

								if(r_sq > h_sq)
								{
									++particle_ptr;
									continue;
								}

								density += c::particleMass*kernel::W_poly6(r_sq, h_sq, c::H);

								++particle_ptr;
							}
						}
					}
				}

				xyzw_data[idx] = glm::vec4(cell_vertex_position, density);
			}
		}
	}

	int no_triangles = 0;
	float minValue = 15.0f;

	auto triangles = MarchingCubes(c::voxelGridDimension, c::voxelGridDimension, c::voxelGridDimension, c::voxelSize, c::voxelSize, c::voxelSize, minValue, std::move(xyzw_data), no_triangles);

	this->no_vertices = no_triangles * 3;

	for(int i = 0; i < no_triangles; ++i)
	for(int j = 0; j < 3; ++j)
	{
		vertices_buffer[i * 3 + j].v = triangles[i].p[j];
		vertices_buffer[i * 3 + j].n = glm::normalize(triangles[i].norm[j]);
	}

	update_buffers();
}


bool MCMesh::exportOBJ(const std::string& filename) const
{
	// Add output directory posix path to file name
	std::string outputDir = "output/";
	std::string fullPath = "";

	// Set the fullPath
	fullPath = outputDir + filename;

	// If the output folder doesn't exist try to create one
	// if creating it was unsuccessful then modify fullPath so it just outputs to the build root
	struct stat sb;
	if(stat(outputDir.c_str(), &sb) != 0 || !S_ISDIR(sb.st_mode)) //dir doesn't exist
	{
		if(_mkdir(outputDir.c_str()) == -1)
		{
			std::cout << "Directory '" << outputDir << "' could not be created! \n" <<
				"The file will not be outputed in the root of the project build dir." <<
				std::endl;

			fullPath = filename;
		}
	}

	// Open the stream and parse
	std::fstream fileOut;
	fileOut.open(fullPath.c_str(), std::ios::out);

	if(fileOut.is_open())
	{
		fileOut << "# CMS Isosurface extraction." << std::endl;
		fileOut << "# George Rassovsky ~ goro.rassovsky@gmail.com \n" << std::endl;

		for(unsigned int i = 0; i < c::voxelGrid3dSize; ++i)
		{
			fileOut << "v " << vertices_buffer[i].v.x << " " << vertices_buffer[i].v.y << " " << vertices_buffer[i].v.z << std::endl;
		}

		// Write vertex normals
		for(unsigned int i = 0; i < c::voxelGrid3dSize; ++i)
		{
			fileOut << "vn " << vertices_buffer[i].n.x << " " << vertices_buffer[i].n.y << " " << vertices_buffer[i].n.z << std::endl;
		}

		char * resolved = 0; // todo -- error handle realpath
		std::cout << "\nExported .py script path: not-realpath \t" << fullPath.c_str() << std::endl; // << realpath(fullPath.c_str(), resolved) << std::endl;
		return true;
	}
	else
	{
		std::cout << "File : " << filename << " Not founds " << std::endl;
		return false;
	}
}

void MCMesh::update_buffers()
{
	// http://stackoverflow.com/questions/15821969/what-is-the-proper-way-to-modify-opengl-vertex-buffer
	// alternatywa: http://stackoverflow.com/questions/7256479/is-it-possible-to-in-place-resize-vbos
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * no_vertices, &this->vertices_buffer[0]);      //replace data in VBO with new data
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MCMesh::loadTextures(void)
{
	// Load and create a texture 
	glGenTextures(1, &_texture);
	glBindTexture(GL_TEXTURE_2D, _texture); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
	// Set our texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// Set texture wrapping to GL_REPEAT (usually basic wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Load, create texture and generate mipmaps
	{
		int width, height;
		unsigned char* image = SOIL_load_image("textures/brickwall.jpg", &width, &height, 0, SOIL_LOAD_RGB);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);
		SOIL_free_image_data(image);
	}
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.

	// Load and create a texture 
	glGenTextures(1, &_normalmap);
	glBindTexture(GL_TEXTURE_2D, _normalmap); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
	// Set our texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// Set texture wrapping to GL_REPEAT (usually basic wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Load, create texture and generate mipmaps
	{
		int width, height;
		unsigned char* image = SOIL_load_image("textures/oilrush1_normal.png", &width, &height, 0, SOIL_LOAD_RGB);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);
		SOIL_free_image_data(image);
	}
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.
}