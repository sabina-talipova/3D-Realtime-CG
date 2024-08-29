
// glm
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// project
#include "cgra/cgra_geometry.hpp"
#include "skeleton_model.hpp"


using namespace std;
using namespace glm;
using namespace cgra;


void skeleton_model::draw(const mat4 &view, const mat4 &proj) {
	// set up the shader for every draw call
	glUseProgram(shader);
	glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));

	// if the skeleton is not empty, then draw
	if (!skel.bones.empty()) {
		drawBone(view, 0);
	}
}


void skeleton_model::drawBone(const mat4 &parentTransform, int boneid) {

    skeleton_bone& bone = skel.bones[boneid];

    glm::mat4 boneTransform = parentTransform * translate(mat4(1), bone.direction * (bone.length * 50));

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
	mat4 scale_mat = scale(mat4(1), vec3(1.0f));
	scale_mat = parentTransform * scale_mat;
	vec3 color = vec3(0.5, 0.5, 0.5);

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
	mat4 scale_mat = scale(mat4(1), vec3(0.5, 0.5, bone.length * 50));
	vec3 color = vec3(0.5, 0.5, 0.5);

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

	vec3 x_color = vec3(0, 1, 0);
	vec3 y_color = vec3(1, 0, 0);
	vec3 z_color = vec3(0, 0, 1);

	mat4 scale_matrix = scale(mat4(1), vec3(0.1f, 0.1f, 2.0f));

	mat4 x_view_matrix = parentTransform * x_rotation * scale_matrix;
	mat4 y_view_matrix = parentTransform * y_rotation * scale_matrix;
	mat4 z_view_matrix = parentTransform * z_rotation * scale_matrix;

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
