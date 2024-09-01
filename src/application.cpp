
// std
#include <iostream>
#include <string>
#include <chrono>

// glm
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

// project
#include "application.hpp"
#include "cgra/cgra_geometry.hpp"
#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_image.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_wavefront.hpp"


using namespace std;
using namespace cgra;
using namespace glm;

float animationTime = 0.0f;
float totalTime = 10.0f;

float lastTime = 0.0f;


void basic_model::draw(const glm::mat4 &view, const glm::mat4 proj) {
	mat4 modelview = view * modelTransform;
	
	glUseProgram(shader); // load shader and variables
	glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(modelview));
	glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(color));

	mesh.draw(); // draw
}

void basic_model::animate(const glm::mat4& view, const glm::mat4 proj, float deltaTime)
{
	animationTime += deltaTime;
	if (animationTime > totalTime) {
		animationTime = 0.0f;
	}

	float t = animationTime / totalTime;
	int numPoints = animation_path.points.size();
	int segment = int(t * (numPoints - 3));
	float localT = (t * (numPoints - 3)) - segment;
	std::vector<glm::vec3> controlPoints = animation_path.points;


	glm::vec3 interpolatedPosition = animation_path.calculateCatmullRomPoint(
		controlPoints[segment],
		controlPoints[segment + 1],
		controlPoints[segment + 2],
		controlPoints[segment + 3],
		localT
	);

	glm::mat4 modelMatrix = view * glm::translate(glm::mat4(1.0f), interpolatedPosition) * modelTransform;

	glUseProgram(shader);
	glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(modelMatrix));
	glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(color));

	mesh.draw();
}


Application::Application(GLFWwindow *window) : m_window(window) {
	
	shader_builder sb;
    sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
	sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_frag.glsl"));
	GLuint shader = sb.build();

	m_model.shader = shader;
	m_model.mesh = load_wavefront_data(CGRA_SRCDIR + std::string("/res//assets//teapot.obj")).build();
	m_model.color = vec3(1, 0, 0);

	sk_model.shader = shader;
	sk_model.skel = skeleton_data(CGRA_SRCDIR + std::string("/res//assets//priman.asf"));

	sp_model.shader = shader;

	const glm::vec3 verts[] = {
		glm::vec3(-30, -10, -20),
		glm::vec3(30, 10, -10),
		glm::vec3(-20, -5, -5),
		glm::vec3(20, 5, 1),
		glm::vec3(-10, 0, 5),
		glm::vec3(10, 0, 10),
		glm::vec3(-40, -5, 20),
		glm::vec3(40, 5, 30)
	};

	for (auto vert : verts) {
		sp_model.points.push_back(vert);
	}

	m_model.animation_path = sp_model;
}


void Application::render() {
	float currentTime = static_cast<float>(glfwGetTime());
	float deltaTime = currentTime - lastTime;
	lastTime = currentTime;
	
	// retrieve the window hieght
	int width, height;
	glfwGetFramebufferSize(m_window, &width, &height); 

	m_windowsize = vec2(width, height); // update window size
	glViewport(0, 0, width, height); // set the viewport to draw to the entire window

	// clear the back-buffer
	glClearColor(0.3f, 0.3f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

	// enable flags for normal/forward rendering
	glEnable(GL_DEPTH_TEST); 
	glDepthFunc(GL_LESS);

	// projection matrix
	mat4 proj = perspective(1.f, float(width) / height, 0.1f, 1000.f);

	// view matrix
	mat4 view = translate(mat4(1), vec3(0, 0, -m_distance))
		* rotate(mat4(1), m_pitch, vec3(1, 0, 0))
		* rotate(mat4(1), m_yaw,   vec3(0, 1, 0));


	// helpful draw options
	if (m_show_grid) drawGrid(view, proj);
	if (m_show_axis) drawAxis(view, proj);
	glPolygonMode(GL_FRONT_AND_BACK, (m_showWireframe) ? GL_LINE : GL_FILL);


	// draw the model
	//m_model.draw(view, proj);

	// draw skeleton
	if (m_show_skeleton) sk_model.draw(view, proj);

	// draw spline
	if (m_show_spline) {
		if (m_show_bezier_spline) {
			sp_model.show_bezier_curve = true;
			sp_model.show_catmull_rom_curve = false;
		}
		else if (m_show_catmull_rom_spline) {
			sp_model.show_catmull_rom_curve = true;
			sp_model.show_bezier_curve = false;
		}
		else {
			sp_model.show_bezier_curve = false;
			sp_model.show_catmull_rom_curve = false;
		}
		sp_model.draw(view, proj);
	}

	if (m_animate_object) {
		m_model.animate(view, proj, deltaTime);
	}
}


void Application::renderGUI() {

	// setup window
	ImGui::SetNextWindowPos(ImVec2(5, 5), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 600), ImGuiSetCond_Once);
	ImGui::Begin("Options", 0);

	// display current camera parameters
	ImGui::Text("Application %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::SliderFloat("Pitch", &m_pitch, -pi<float>() / 2, pi<float>() / 2, "%.2f");
	ImGui::SliderFloat("Yaw", &m_yaw, -pi<float>(), pi<float>(), "%.2f");
	ImGui::SliderFloat("Distance", &m_distance, 0, 100, "%.2f", 2.0f);

	// helpful drawing options
	ImGui::Checkbox("Show axis", &m_show_axis);
	ImGui::SameLine();
	ImGui::Checkbox("Show grid", &m_show_grid);
	ImGui::Checkbox("Wireframe", &m_showWireframe);
	ImGui::SameLine();
	if (ImGui::Button("Screenshot")) rgba_image::screenshot(true);

	
	ImGui::Separator();

	// example of how to use input boxes
	static float exampleInput;
	if (ImGui::InputFloat("example input", &exampleInput)) {
		cout << "example input changed to " << exampleInput << endl;
	}

	// Skeleton helpers
	ImGui::Separator();
	ImGui::Text("Skeleton");
	ImGui::Checkbox("Show skeleton", &m_show_skeleton);

	ImGui::Text("Skeleton Pose");

	if (ImGui::Button("Default")) sk_model.skel = skeleton_data(CGRA_SRCDIR + std::string("/res//assets//priman.asf"));
	ImGui::SameLine();
	if (ImGui::Button("Kicking")) sk_model.skel = skeleton_data(CGRA_SRCDIR + std::string("/res//assets//priman.asf"));
	ImGui::SameLine();
	if (ImGui::Button("Running")) sk_model.skel = skeleton_data(CGRA_SRCDIR + std::string("/res//assets//priman.asf"));

	if (ImGui::Button("Punching")) sk_model.skel = skeleton_data(CGRA_SRCDIR + std::string("/res//assets//priman.asf"));
	ImGui::SameLine();
	if (ImGui::Button("Walking")) sk_model.skel = skeleton_data(CGRA_SRCDIR + std::string("/res//assets//walking.asf"));
	ImGui::SameLine();
	if (ImGui::Button("Seating")) sk_model.skel = skeleton_data(CGRA_SRCDIR + std::string("/res//assets//priman.asf"));

	ImGui::Separator();

	ImGui::Text("Spline Points");
	ImGui::Separator();
	ImGui::Checkbox("Show spline", &m_show_spline);

	if(ImGui::Checkbox("Show bezier spline", &m_show_bezier_spline)) m_show_catmull_rom_spline = false;
	ImGui::SameLine();
	if(ImGui::Checkbox("Show catmull rom spline", &m_show_catmull_rom_spline)) m_show_bezier_spline = false;
	ImGui::Separator();

	if (ImGui::Button("Add point")) {
		sp_model.points.push_back(vec3(0, 0, 0));
	}
	for (int i = 0; i < (int)sp_model.points.size(); i++) {
		ImGui::SliderFloat3(("##" + to_string(i)).c_str(), value_ptr(sp_model.points[i]), -100, 100);
		ImGui::SameLine();
		if (ImGui::Button(("-##" + to_string(i)).c_str())) {
			sp_model.points.erase(sp_model.points.begin() + i);
		}
	}

	ImGui::Separator();

	ImGui::Text("Animate Object");
	if (ImGui::Button("Play")) {
		m_animate_object = !m_animate_object;
		m_show_spline = !m_show_spline;
	}

	// finish creating window
	ImGui::End();
}


void Application::cursorPosCallback(double xpos, double ypos) {
	if (m_leftMouseDown) {
		vec2 whsize = m_windowsize / 2.0f;

		// clamp the pitch to [-pi/2, pi/2]
		m_pitch += float(acos(glm::clamp((m_mousePosition.y - whsize.y) / whsize.y, -1.0f, 1.0f))
			- acos(glm::clamp((float(ypos) - whsize.y) / whsize.y, -1.0f, 1.0f)));
		m_pitch = float(glm::clamp(m_pitch, -pi<float>() / 2, pi<float>() / 2));

		// wrap the yaw to [-pi, pi]
		m_yaw += float(acos(glm::clamp((m_mousePosition.x - whsize.x) / whsize.x, -1.0f, 1.0f))
			- acos(glm::clamp((float(xpos) - whsize.x) / whsize.x, -1.0f, 1.0f)));
		if (m_yaw > pi<float>()) m_yaw -= float(2 * pi<float>());
		else if (m_yaw < -pi<float>()) m_yaw += float(2 * pi<float>());
	}

	// updated mouse position
	m_mousePosition = vec2(xpos, ypos);
}


void Application::mouseButtonCallback(int button, int action, int mods) {
	(void)mods; // currently un-used

	// capture is left-mouse down
	if (button == GLFW_MOUSE_BUTTON_LEFT)
		m_leftMouseDown = (action == GLFW_PRESS); // only other option is GLFW_RELEASE
}


void Application::scrollCallback(double xoffset, double yoffset) {
	(void)xoffset; // currently un-used
	m_distance *= pow(1.1f, -yoffset);
}


void Application::keyCallback(int key, int scancode, int action, int mods) {
	(void)key, (void)scancode, (void)action, (void)mods; // currently un-used
}


void Application::charCallback(unsigned int c) {
	(void)c; // currently un-used
}
