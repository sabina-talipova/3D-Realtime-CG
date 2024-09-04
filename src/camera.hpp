#pragma once
// standard libs
#include <vector>
#include <algorithm>

// glm
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// project
#include "cgra/cgra_geometry.hpp"
#include "spline_model.hpp"

using namespace std;
using namespace glm;

class camera
{
public:
	glm::vec3 position;
	glm::vec3 target;
	glm::vec3 up;
	std::vector<glm::vec3> camera_path;

	glm::vec3 calculateCatmullRomPoint(const std::vector<glm::vec3>& points, float t) {
		int numPoints = points.size();
		int p0, p1, p2, p3;

		if (numPoints < 4) return glm::vec3(0.0f);

		float f = t * (numPoints - 3);
		int intF = static_cast<int>(f);
		float localT = f - intF;

		p0 = (intF - 1 + numPoints) % numPoints;
		p1 = intF;
		p2 = (intF + 1) % numPoints;
		p3 = (intF + 2) % numPoints;

		glm::vec3 a = -0.5f * points[p0] + 1.5f * points[p1] - 1.5f * points[p2] + 0.5f * points[p3];
		glm::vec3 b = points[p0] - 2.5f * points[p1] + 2.0f * points[p2] - 0.5f * points[p3];
		glm::vec3 c = -0.5f * points[p0] + 0.5f * points[p2];
		glm::vec3 d = points[p1];

		return a * localT * localT * localT + b * localT * localT + c * localT + d;
	}

	void animateCamera(const glm::vec3& objectPosition, float t) {
		int numPoints = camera_path.size();
		int segment = int(t * (numPoints - 3));
		float localT = (t * (numPoints - 3)) - segment;

		glm::vec3 cameraPosition = calculateCatmullRomPoint(camera_path, localT);
		position = cameraPosition;
		target = objectPosition;

		up = glm::vec3(0.0f, 1.0f, 0.0f);
	}
};
