#pragma once
#include <vector>
#include "Node.h"
#include <glm\glm.hpp>

class GroupNode : public Node
{
	std::vector<Node*> nodes_;
public:

	explicit GroupNode(const std::string& name);
	~GroupNode();

	void init(RenderingEngine* rendering_engine) override;

	std::vector<IDrawable*> get_drawables() override;
	std::vector<IDrawable*> get_transparent_drawables() override;
	std::vector<LightNode*> get_light_nodes() override;
	std::vector<AnimatorNode*> get_animator_nodes() override;
	std::vector<ParticleEmitterNode*> get_particle_emitter_nodes() override;

	const std::vector<Node*>& get_nodes() const;
	void add_node(Node *node);

	void apply_transformation(const glm::mat4& transformation, const glm::mat4& inverse_transformation) override;

	Node* find_by_name(const std::string& name) override;

	virtual void set_enabled(bool enabled) override;
};

