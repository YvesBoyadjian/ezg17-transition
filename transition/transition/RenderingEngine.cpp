#include "RenderingEngine.h"
#include "RenderingNode.h"
#include "glheaders.h"
#include <iostream>
#include "GroupNode.h"
#include "IResource.h"
#include "MainShader.h"
#include "GLDebugContext.h"
#include "Node.h"

RenderingEngine::RenderingEngine(const glm::ivec2 viewport)
{
	this->root_node_ = new GroupNode("root");
	this->viewport_ = viewport;

	this->main_shader_ = new MainShader();
	this->register_resource(this->main_shader_);
}

RenderingEngine::~RenderingEngine()
{
	delete this->root_node_;
}

void RenderingEngine::register_resource(IResource* resource)
{
	this->resources_.push_back(resource);
}

void error_callback(int error, const char* description)
{
	std::cout << "Error: " << std::string(description) << std::endl;
}

void RenderingEngine::run()
{
	glfwSetErrorCallback(error_callback);

	glfwInit();

#if _DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	const auto window = glfwCreateWindow(this->viewport_.x, this->viewport_.y, "Transition", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(window);


	if (!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress)))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		glfwTerminate();
		return;
	}

	//Set DebugContext Callback
#if _DEBUG
	// Query the OpenGL function to register your callback function.
	const PFNGLDEBUGMESSAGECALLBACKPROC _glDebugMessageCallback = PFNGLDEBUGMESSAGECALLBACKPROC(glfwGetProcAddress("glDebugMessageCallback"));

	// Register your callback function.
	if (_glDebugMessageCallback != nullptr) {
		_glDebugMessageCallback(DebugCallback, nullptr);
	}

	// Enable synchronous callback. This ensures that your callback function is called
	// right after an error has occurred. 
	if (_glDebugMessageCallback != nullptr) {
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}
#endif

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	for (auto& resource : resources_)
	{
		resource->init();
	}

	this->root_node_->init(this);

	this->drawables_ = this->root_node_->get_drawables();
	this->rendering_nodes_ = this->root_node_->get_rendering_nodes();
	this->light_nodes_ = this->root_node_->get_light_nodes();

	float currentscale = 1.0f;
	float scalemult = 1.005f;
	Node* gitti = this->root_node_->find_by_name("Sphere");
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	while (!glfwWindowShouldClose(window))
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, true);
		}

		for (auto& rendering_node : this->rendering_nodes_)
		{
			rendering_node->render(this->drawables_, this->light_nodes_);
		}

		gitti->apply_transformation(Transformation::scale(glm::vec3(scalemult, 1.0f, 1.0f)));
		currentscale *= scalemult;
		if ((currentscale > 3.0f && scalemult > 1.0f) || (currentscale < 0.5f && scalemult < 1.0f)) {
			scalemult = 1 / scalemult;
		}
		std::cout << currentscale << std::endl;
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();

	for (auto& resource : resources_) {
		delete resource;
	}
}
