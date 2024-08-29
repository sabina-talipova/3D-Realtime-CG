#pragma once

// glm
#include <glm/glm.hpp>

// project
#include "opengl.hpp"
#include "skeleton.hpp"

class skeleton_model {
private:
	// recursive helper method
	void drawBone(const glm::mat4 &view, int boneid);
	void drawJoint(const glm::mat4& parentTransform);
	void drawMainBone(const glm::mat4& parentTransform, skeleton_bone& bone);
	void drawAxis(const glm::mat4& parentTransform, skeleton_bone& bone);
	void drawAxisHelper(const glm::vec3 color, glm::mat4& view_matrix);

public:
	GLuint shader = 0;
	skeleton_data skel;
	skeleton_pose pose;

	skeleton_model() { }
	void draw(const glm::mat4 &view, const glm::mat4 &proj);
};