#pragma once
#include "IResource.h"
#include "glheaders.h"
#include "Material.h"

class MeshResource : public IResource
{
	float *vertices_ = nullptr;
	float *normals_ = nullptr;
	float *uvs_ = nullptr;
	int num_vertices_;

	unsigned int *indices_;
	int num_indices_;
	GLuint vao_;
	GLuint vbo_positions_;
	GLuint vbo_normals_;
	GLuint vbo_uvs_;
	GLuint ebo_;

	Material material_;

public:
	static MeshResource *create_cube(glm::vec3 color);
	static MeshResource *create_sprite(TextureRenderable *resource);
	static MeshResource *create_sprite(TextureRenderable *resource, TextureRenderable *alpha, bool switch_uv);

	MeshResource(float *vertices, float *normals, float *uvs, int num_vertices, unsigned int *indices, int num_indices, const Material& material);
	~MeshResource();

	int get_resource_id() const override;
	void init() override;

	//calculates the radius of a sphere with center at the origin which contains all vertices
	float calculate_sphere_radius(const glm::mat4& trafo) const;

	int get_num_indices() const
	{
		return num_indices_;
	}

	int get_num_vertices() const
	{
		return num_vertices_;
	}

	const Material& get_material() const {
		return material_;
	}

	Material& get_editable_material() {
		return material_;
	}
};

