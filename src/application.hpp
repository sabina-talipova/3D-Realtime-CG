
#pragma once

// glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// project
#include "opengl.hpp"
#include "cgra/cgra_mesh.hpp"
#include "skeleton_model.hpp"
#include "spline_model.hpp"
#include "camera.hpp"


// Basic model that holds the shader, mesh and transform for drawing.
// Can be copied and modified for adding in extra information for drawing
// including textures for texture mapping etc.
struct basic_model {
	GLuint shader = 0;
	cgra::gl_mesh mesh;
	glm::vec3 color{0.7};
	glm::mat4 modelTransform{1.0};
	GLuint texture;

	spline_model animation_path;

	glm::vec3 interpolatedPosition;

	void draw(const glm::mat4 &view, const glm::mat4 proj);
	void animate(const glm::mat4& view, const glm::mat4 proj);
	void calculateCatmullRomPoint();
};

// Main application class
//
class Application {
private:
	// window
	glm::vec2 m_windowsize;
	GLFWwindow *m_window;

	// oribital camera
	float m_pitch = .86;
	float m_yaw = -.86;
	float m_distance = 20;

	// last input
	bool m_leftMouseDown = false;
	glm::vec2 m_mousePosition;

	// drawing flags
	bool m_show_axis = false;
	bool m_show_grid = false;
	bool m_showWireframe = false;

	// skeleton project flags
	bool m_show_skeleton = false;

	// spline project flags
	bool m_show_spline = false;
	bool m_show_bezier_spline = false;
	bool m_show_catmull_rom_spline = false;

	bool m_animate_object = false;
	bool m_animate_camera = false;

	// geometry
	basic_model m_model;

	// skeleton data & geometry
	skeleton_model sk_model;

	// spline
	spline_model sp_model;

	//camera
	camera m_camera;
	float m_totalTime = 24.0f;

public:
	// setup
	Application(GLFWwindow *);

	// disable copy constructors (for safety)
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	// rendering callbacks (every frame)
	void render();
	void renderGUI();

	// input callbacks
	void cursorPosCallback(double xpos, double ypos);
	void mouseButtonCallback(int button, int action, int mods);
	void scrollCallback(double xoffset, double yoffset);
	void keyCallback(int key, int scancode, int action, int mods);
	void charCallback(unsigned int c);
};