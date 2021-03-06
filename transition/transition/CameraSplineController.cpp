#include "CameraSplineController.h"
#include "TransformationNode.h"
#include "ColladaImporter.h"
#include "GeometryNode.h"
#include <spline_library/spline.h>
#include <spline_library/vector.h>
#include <spline_library/splines/uniform_cr_spline.h>
#include <stdlib.h>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/matrix_decompose.hpp>

glm::quat look_at(glm::vec3 start, glm::vec3 dest) {
	start = glm::normalize(start);
	dest = glm::normalize(dest);

	const float cos_theta = dot(start, dest);
	glm::vec3 rotation_axis;

	if (cos_theta < -1 + 0.001f) {
		// special case when vectors in opposite directions:
		// there is no "ideal" rotation axis
		// So guess one; any will do as long as it's perpendicular to start
		rotation_axis = cross(glm::vec3(0.0f, 0.0f, 1.0f), start);
		if (glm::length2(rotation_axis) < 0.01) // bad luck, they were parallel, try again!
			rotation_axis = cross(glm::vec3(1.0f, 0.0f, 0.0f), start);

		rotation_axis = normalize(rotation_axis);
		return glm::angleAxis(glm::radians(180.0f), rotation_axis);
	}

	rotation_axis = cross(start, dest);

	float s = sqrt((1 + cos_theta) * 2);
	float invs = 1 / s;

	return glm::quat(
		s * 0.5f,
		rotation_axis.x * invs,
		rotation_axis.y * invs,
		rotation_axis.z * invs
	);
}

CameraSplineController::CameraSplineController(std::string name, TransformationNode* target, GroupNode *visualizer_container) : AnimatorNode(name)
{
	this->target_ = target;
	this->progress_ = 0.0;
	this->position_spline_ = nullptr;
	this->duration_ = 0.0;

#ifdef VISUALIZE_KEYPOINTS
	this->keypoint_visualizer_ = MeshResource::create_cube(glm::vec3(0, 0, 1));
	this->cam_visualizer_ = MeshResource::create_cube(glm::vec3(1, 0, 0));
	this->visualizer_container_ = visualizer_container;
	this->test_cam_ = nullptr;
#endif

}

CameraSplineController::~CameraSplineController()
{
	for (auto &keypoint : this->keypoints_)
	{
		// TODO: delete also actions, but they are shared among keypoints
		delete keypoint;
	}
}

void CameraSplineController::add_keypoint(KeyPoint* key_point)
{
	this->keypoints_.push_back(key_point);
}

void CameraSplineController::update(double delta)
{
	// position
	float tween = this->progress_ / this->duration_ * this->position_spline_->getMaxT();

	const Vector3 interpolated_pos = this->position_spline_->getPosition(tween);
	const glm::vec3 new_pos = glm::vec3(interpolated_pos[0], interpolated_pos[1], interpolated_pos[2]);
	
	// rotation
	KeyPoint *current_keypoint = this->keypoints_[int(tween) + 1];
	KeyPoint *next_keypoint = this->keypoints_[int(tween) + 1];
	glm::vec3 target_look_at = current_keypoint->look_at_pos; // this->target_->get_position();  
	//std::cout << "Look At " << target_look_at.x << " " << target_look_at.y << " " << target_look_at.z << std::endl;

	glm::vec3 target_direction = glm::normalize(target_look_at - this->get_target()->get_position());

	glm::mat4 new_rotation;
	float global_tween = (tween - next_keypoint->at_time) / float(next_keypoint->duration / this->duration_ * this->position_spline_->getMaxT());
	if (glm::length(target_direction) >= 0.00001)
	{
		glm::mat4 target_rotation = glm::inverse(glm::lookAt(this->get_target()->get_position(), target_look_at, glm::vec3(0, 1, 0)));
		glm::quat quat_target = glm::quat_cast(target_rotation);

		if (glm::dot(this->current_rotation_, quat_target) < 0.0)
		{
			quat_target = -quat_target;
		}

		this->current_rotation_ = glm::mix(this->current_rotation_, quat_target, global_tween*global_tween);

	}
	
	glm::mat4 mat = glm::translate(new_pos) * glm::mat4_cast(this->current_rotation_);
	get_target()->set_transformation(mat);


	if (this->progress_ < this->duration_) {
		this->progress_ += delta;
	}
	this->progress_ = std::min(this->progress_, this->duration_);

	for (auto &action : current_keypoint->actions)
	{
		action->update(this, delta, global_tween);
	}
	
#ifdef DEBUG_KEYS
	if (glfwGetKey(this->get_rendering_engine()->get_window(), GLFW_KEY_M))
	{
		this->progress_ = 0;
	}
#endif
}

void CameraSplineController::init(RenderingEngine* rendering_engine)
{
	AnimatorNode::init(rendering_engine);
	const auto interpolated_pos = position_spline_->getPosition(0);
	const glm::vec3 new_pos = glm::vec3(interpolated_pos[0], interpolated_pos[1], interpolated_pos[2]);
	auto mat = glm::lookAt(new_pos, this->keypoints_[1]->look_at_pos, glm::vec3(0, 1, 0));
	get_target()->set_transformation(glm::inverse(mat), mat);


	glm::vec3 forward = glm::normalize(glm::transpose(get_target()->get_inverse_transformation()) * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
	this->current_rotation_ = glm::quat_cast(glm::inverse(glm::lookAt(glm::vec3(0, 0, 0), forward, glm::vec3(0, 1, 0))));

#ifdef VISUALIZE_KEYPOINTS
	this->keypoint_visualizer_->init();
	rendering_engine->register_resource(this->keypoint_visualizer_);
	this->cam_visualizer_->init();
	rendering_engine->register_resource(this->cam_visualizer_);
#endif
}

void CameraSplineController::build_spline()
{
	std::cout << "Building Spline..." << std::endl;

	this->duration_ = 0;
	std::vector<Vector3> timeless_position_spline_points;
	for (auto &point : this->keypoints_)
	{
		point->at_time = this->duration_;
		this->duration_ += point->duration;

		timeless_position_spline_points.push_back(Vector3({ point->pos.x, point->pos.y, point->pos.z }));
	}
	// remove both of the last 2 points
	auto last = this->keypoints_[this->keypoints_.size() - 1];
	auto nexttolast = this->keypoints_[this->keypoints_.size() - 2];
	auto first = this->keypoints_[0];
	this->duration_ -= first->duration + last->duration + nexttolast->duration;

	UniformCRSpline<Vector3> timeless_position_spline = UniformCRSpline<Vector3>(timeless_position_spline_points);

	std::vector<KeyPoint*> new_keypoints;
	new_keypoints.push_back(first);
	std::vector<Vector3> position_spline_points;
	position_spline_points.push_back(Vector3({ first->pos.x, first->pos.y, first->pos.z }));
	for (int i = 0; i <= timeless_position_spline.getMaxT(); i++)
	{
		auto keypoint = this->keypoints_[i + 1];
		for (int time = 1; time <= keypoint->duration; time++)
		{ 
			auto tweened_pos = timeless_position_spline.getPosition(i + float(time) / keypoint->duration);
			position_spline_points.push_back(tweened_pos);

			auto kp = new KeyPoint(keypoint);
			kp->pos = glm::vec3(tweened_pos[0], tweened_pos[1], tweened_pos[2]);
			new_keypoints.push_back(kp);
		}
	}
	position_spline_points.push_back(Vector3({ last->pos.x, last->pos.y, last->pos.z }));
	new_keypoints.push_back(last);
	this->keypoints_ = new_keypoints;

	position_spline_ = new UniformCRSpline<Vector3>(position_spline_points);

#ifdef VISUALIZE_KEYPOINTS
	for (auto &point : this->keypoints_)
	{
		GeometryNode *node = new GeometryNode("keypoint" + std::to_string(this->keypoints_.size()), this->keypoint_visualizer_);
		node->set_transformation(glm::translate(point->pos) * glm::scale(glm::vec3(0.25, 0.25, 0.25)));
		this->visualizer_container_->add_node(node);
	}

	this->test_cam_ = new GeometryNode("keypoint", this->keypoint_visualizer_);
	this->visualizer_container_->add_node(this->test_cam_);

#endif
	std::cout << "Finished building Spline..." << std::endl;
}

TransformationNode* CameraSplineController::get_target() const
{
#ifdef VISUALIZE_KEYPOINTS
	return test_cam_ && ! glfwGetKey(this->get_rendering_engine()->get_window(), GLFW_KEY_F12) ? test_cam_ : this->target_;
#else
		return this->target_;
#endif
}
