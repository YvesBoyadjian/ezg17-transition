#pragma once
#include "Node.h"

class AnimatorNode: public Node
{
public:
	explicit AnimatorNode(const std::string& name)
		: Node(name)
	{

	}
	virtual void apply_transformation(const glm::mat4& transformation, const glm::mat4& inverse_transformation) override
	{
		// do nothing
	}

	std::vector<AnimatorNode*> get_animator_nodes() override
	{
		return{ this };
	}

	virtual void update(double delta) = 0;
};

