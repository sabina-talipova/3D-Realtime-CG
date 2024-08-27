
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

    const skeleton_bone& bone = skel.bones[boneid];

    glm::mat4 boneTransform = parentTransform * translate(mat4(1), bone.direction * (bone.length * 50));
    boneTransform *= glm::mat4_cast(glm::quat(bone.basis));

	if (boneid) {
		drawJoint(parentTransform);
		//drawMainBone(parentTransform, bone);
		//drawAxis(parentTransform, bone);
	}
	


    for (int childIndex : bone.children) {
        drawBone(boneTransform, childIndex);
    }
}

void skeleton_model::drawJoint(const mat4& parentTransform)
{
}

void skeleton_model::drawMainBone(const glm::mat4& parentTransform, skeleton_bone& bone)
{
}

void skeleton_model::drawAxis(const glm::mat4& parentTransform, skeleton_bone& bone)
{
}

void skeleton_model::drawAxisHelper(const glm::vec3 color, glm::mat4& rotation)
{
}
