#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// glm
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// project
#include "cgra/cgra_geometry.hpp"
#include "skeleton_model.hpp"


using namespace std;
using namespace glm;
using namespace cgra;


#define BONE_SCALE 30

struct Pose {
	string bone_name;
	glm::vec3 new_bone_axis;
	glm::vec3 new_bone_position;
};

vector<Pose> new_pose;

glm::mat4 createRotationMatrix(float pitch, float yaw, float roll) {
	glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f), glm::radians(pitch), glm::vec3(1, 0, 0));
	glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f), glm::radians(yaw), glm::vec3(0, 1, 0));
	glm::mat4 rotateZ = glm::rotate(glm::mat4(1.0f), glm::radians(roll), glm::vec3(0, 0, 1));

	return rotateY * rotateX * rotateZ;
}

glm::vec3 applyRotation(const glm::vec3& direction, float pitch, float yaw, float roll) {
	glm::mat4 rotationMatrix = createRotationMatrix(pitch, yaw, roll);
	glm::vec4 newDirection = rotationMatrix * glm::vec4(direction, 0.0f);
	return glm::vec3(newDirection);
}

void skeleton_model::parseFile(const std::string& filename) {
	std::ifstream file(filename);

	if (!file.is_open()) {
		std::cerr << "Cannot open file : " << filename << std::endl;
		return;
	}

	std::string line;
	while (std::getline(file, line)) {
		istringstream iss(line);
		string token;
		vector<string> tokens;
		Pose pose;

		while (iss >> token) {
			tokens.push_back(token);
		}

		int vec_size = tokens.size();

		pose.bone_name = tokens[0];

		int boneId = skel.findBone(pose.bone_name);

		if (boneId >= 0) {
			skeleton_bone bone = skel.bones[boneId];
			glm::vec3 old_direction = bone.direction;

			glm::vec3 anglesInDegrees = glm::degrees(bone.basis);

			float x = anglesInDegrees.x;
			float y = anglesInDegrees.y;
			float z = anglesInDegrees.z;

			if (bone.freedom & dof_rx) {
				x = std::stof(tokens[1]);
			}

			if (bone.freedom & dof_ry) {
				y = (vec_size == 3) ? std::stof(tokens[vec_size - 2]) : std::stof(tokens[vec_size - 1]);
			}

			if (bone.freedom & dof_rz) {
				z = std::stof(tokens[vec_size - 1]);
			}

			pose.new_bone_axis = vec3(x, y, z);
			pose.new_bone_position = applyRotation(old_direction, x, y, z);

			std::cout << "pose_name " << pose.bone_name << std::endl;
			std::cout << "direction " << pose.new_bone_position.x << " " << pose.new_bone_position.y << " " << pose.new_bone_position.z << std::endl;
			std::cout << "axis " << pose.new_bone_axis.x << " " << pose.new_bone_axis.y << " " << pose.new_bone_axis.z << "   XYZ" << std::endl;

			
		}

		new_pose.push_back(pose);
	}

	file.close();
}

void skeleton_model::computeNewDirections() {

	parseFile(CGRA_SRCDIR + std::string("/res//assets//test.txt"));
}

void skeleton_model::draw(const mat4 &view, const mat4 &proj) {
	// set up the shader for every draw call
	glUseProgram(shader);
	glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));

	// if the skeleton is not empty, then draw
	if (!skel.bones.empty()) {
		drawBone(view, 0);
	}

	//computeNewDirections();
}


void skeleton_model::drawBone(const mat4 &parentTransform, int boneid) {

    skeleton_bone& bone = skel.bones[boneid];

    glm::mat4 boneTransform = parentTransform * translate(mat4(1), bone.direction * (bone.length * BONE_SCALE));

	if (boneid) {
		drawJoint(parentTransform);
		drawMainBone(parentTransform, bone);
		drawAxis(parentTransform, bone);
	}

    for (int childIndex : bone.children) {
        drawBone(boneTransform, childIndex);
    }
}

void skeleton_model::drawJoint(const mat4& parentTransform)
{
	mat4 scale_mat = scale(mat4(1), vec3(0.8f));
	scale_mat = parentTransform * scale_mat;
	vec3 color = vec3(0.0f, 1.0f, 1.0f);

	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(scale_mat));
	glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(color));

	drawSphere();
}

void skeleton_model::drawMainBone(const glm::mat4& parentTransform, skeleton_bone& bone)
{
	vec3 direction = bone.direction;
	vec3 axis = cross(vec3(0, 0, 1), direction);
	float angle = acos(dot(vec3(0, 0, 1), direction));
	mat4 rotation_mat= rotate(mat4(1), angle, axis);
	mat4 scale_mat = scale(mat4(1), vec3(0.5f, 0.5f, bone.length * BONE_SCALE));
	vec3 color = vec3(0.5f, 0.5f, 0.5f);

	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr((parentTransform * rotation_mat) * scale_mat));
	glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(color));

	drawCylinder();
}

void skeleton_model::drawAxis(const glm::mat4& parentTransform, skeleton_bone& bone)
{
	mat4 rotation = rotate(rotate(rotate(parentTransform, bone.basis.z, vec3(0, 0, 1)), bone.basis.y, vec3(0, 1, 0)), bone.basis.x, vec3(1, 0, 0));

	mat4 x_rotation = rotate(mat4(1), pi<float>() / 2.0f, vec3(0, 1, 0));
	mat4 y_rotation = rotate(mat4(1), (pi<float>() / 2.0f) * -1, vec3(1, 0, 0));
	mat4 z_rotation = rotate(mat4(1), pi<float>() / 2.0f, vec3(0, 0, 1));

	vec3 x_color = vec3(1, 0, 0);
	vec3 y_color = vec3(0, 1, 0);
	vec3 z_color = vec3(0, 0, 1);

	mat4 scale_matrix = scale(mat4(1), vec3(0.1f, 0.1f, 2.0f));

	mat4 x_view_matrix = rotation * x_rotation * scale_matrix;
	mat4 y_view_matrix = rotation * y_rotation * scale_matrix;
	mat4 z_view_matrix = rotation * z_rotation * scale_matrix;

	drawAxisHelper(x_color, x_view_matrix);
	drawAxisHelper(y_color, y_view_matrix);
	drawAxisHelper(z_color, z_view_matrix);
}

void skeleton_model::drawAxisHelper(const glm::vec3 color, glm::mat4& view_matrix)
{
	glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(color));
	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(view_matrix));
	drawCylinder();

	glm::mat4 cone_position = glm::translate(view_matrix, glm::vec3(0.0f, 0.0f, 1.0f));
	cone_position = glm::scale(cone_position, glm::vec3(2.5f, 2.5f, 0.25f));

	glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(color));
	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(cone_position));
	drawCone();
}
