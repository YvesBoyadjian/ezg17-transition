#pragma once
#include <vector>
#include <string>

class IDrawable;
class RenderingEngine;
class RenderingNode;
class LightNode;

class Node
{
	Node *parent_;
	RenderingEngine *rendering_engine_;
	std::string name_;

public:
	explicit Node(const std::string& name);
	virtual ~Node();

	virtual void init(RenderingEngine *rendering_engine);

	virtual std::vector<IDrawable*> get_drawables();
	virtual std::vector<RenderingNode*> get_rendering_nodes();
	virtual std::vector<LightNode*> get_light_nodes();

	RenderingEngine *get_rendering_engine() const
	{
		return this->rendering_engine_;
	}

	void set_parent(Node *parent)
	{
		this->parent_ = parent;
	}

	Node *get_parent() const
	{
		return this->parent_;
	}

	std::string get_name() const
	{
		return this->name_;
	}
};

