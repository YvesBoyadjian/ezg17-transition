#pragma once
#include "TransformationNode.h"
#include <vector>
#include <glm/glm.hpp>
#include "ShaderResource.h"
#include "FrustumG.h"

class AnimatorNode;
class IDrawable;

class RenderingNode :
	public TransformationNode
{
private:
	FrustumG *frustum_;
	bool culling_ = false;

protected:
	glm::ivec2 viewport_;
	glm::mat4 projection_;
	glm::mat4 projection_inv_;

public:
	RenderingNode(const std::string& name, const glm::ivec2 viewport, const float fieldOfView, const float ratio, const float nearp, const float farp, const bool culling);
	RenderingNode(const std::string& name, const glm::ivec2 viewport, const glm::mat4& proj);
	~RenderingNode();

	virtual void before_render(const std::vector<IDrawable*> &drawables, const std::vector<IDrawable*>& transparents, const std::vector<LightNode*> &light_nodes) const;
	virtual void after_render(const std::vector<IDrawable*> &drawables, const std::vector<IDrawable*>& transparents, const std::vector<LightNode*> &light_nodes) const;
	void render(const std::vector<IDrawable*> &drawables, const std::vector<IDrawable*>& transparents, const std::vector<ParticleEmitterNode*> &emitters, const std::vector<LightNode*> &light_nodes) const;

	virtual ShaderResource* get_shader() const = 0;
	virtual bool renders_particles() const { return false; }
	virtual bool is_rendering_enabled() const;
	
	const glm::mat4& get_projection_matrix() const;
	const glm::mat4& get_projection_inverse_matrix() const;
	const glm::mat4& get_view_matrix() const;
	const FrustumG * get_frustum() const;

	void set_view_matrix(const glm::mat4& mat);
	void set_projection_matrix(const glm::mat4& mat);

	void set_viewport(const glm::ivec2 viewport);

	void initialize_culling(const float fieldOfView, const float ratio, const float nearp, const float farp);
};

