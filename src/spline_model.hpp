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

class spline_model
{
public:
    GLuint shader = 0;
    GLuint VAO = 0, VBO = 0;
    std::vector<glm::vec3> points = std::vector<glm::vec3>();
    bool show_bezier_curve = false;
    bool show_catmull_rom_curve = false;

    spline_model() {
        initializeBuffers();
    }

    void initializeBuffers() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void updateVertexData(const std::vector<glm::vec3>& points) {
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        size_t bufferSize = points.size() * sizeof(glm::vec3);
        glBufferData(GL_ARRAY_BUFFER, bufferSize, points.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void draw(const glm::mat4& view, const glm::mat4& proj) {
        glUseProgram(shader);
        glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));

        for (int i = 0; i < (int)points.size(); i++) {
            drawPoint(view, i);
        }

        drawLine(view);
    }

    void drawPoint(const glm::mat4& view, int i) {
        glm::mat4 modelview = view * glm::translate(glm::mat4(1.0f), points[i]);
        glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(modelview * scale(mat4(1.0f), vec3(1, 1, 1))));
        if (i == 0) glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(vec3(0, 1, 0)));
        else if (i == points.size() - 1) glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(vec3(1, 0, 0)));
        else glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(vec3(0.7f)));
        cgra::drawSphere();
    }

    void drawLine(const glm::mat4& view) {
        glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
        glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, glm::value_ptr(glm::vec3(1.0f)));

        updateVertexData(points);

        if (show_bezier_curve) {
            drawBezierCurve(50);
        }
        else if (show_catmull_rom_curve) {
            drawCatmullRomCurve(0.001f);
        }
        else {
            glBindVertexArray(VAO);
            glDrawArrays(GL_LINE_STRIP, 0, points.size());
            glBindVertexArray(0);
        }
    }

    void drawCurve(const std::vector<glm::vec3>& controlPoints) {
        updateVertexData(controlPoints);
        glBindVertexArray(VAO);

        glDrawArrays(GL_LINE_STRIP, 0, controlPoints.size());

        glBindVertexArray(0);
    }

    void drawBezierCurve(int segments) {
        glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, glm::value_ptr(glm::vec3(1.0f)));

        std::vector<glm::vec3> verts;

        for (int i = 0; i <= segments; ++i) {
            float t = (float)i / (float)segments;
            glm::vec3 point = calculateBezierPoint(points, t);
            verts.push_back(point);
        }
        drawCurve(verts);

    }

    glm::vec3 calculateBezierPoint(const std::vector<glm::vec3>& controlPoints, float t) {
        if (controlPoints.size() == 1) {
            return controlPoints[0];
        }

        std::vector<glm::vec3> newPoints;
        for (size_t i = 0; i < controlPoints.size() - 1; ++i) {
            glm::vec3 point = glm::mix(controlPoints[i], controlPoints[i + 1], t);
            newPoints.push_back(point);
        }

        return calculateBezierPoint(newPoints, t);
    }

    void drawCatmullRomCurve(float step = 0.01f) {
        std::vector<glm::vec3> curvePoints;

        if (points.size() < 2) {
            return;
        }

        for (size_t i = 0; i < points.size() - 1; ++i) {
            glm::vec3 p0 = (i == 0) ? points[i] : points[i - 1];
            glm::vec3 p1 = points[i];
            glm::vec3 p2 = points[i + 1];
            glm::vec3 p3 = (i + 2 < points.size()) ? points[i + 2] : points[i + 1];

            for (float t = 0; t <= 1.0f; t += step) {
                glm::vec3 point = calculateCatmullRomPoint(p0, p1, p2, p3, t);
                curvePoints.push_back(point);
            }
        }

        drawCurve(curvePoints);
    }

    glm::vec3 calculateCatmullRomPoint(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t) {
        float t2 = t * t;
        float t3 = t2 * t;

        return 0.5f * (
                (2.0f * p1) +
                (-p0 + p2) * t +
                (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
                (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3
            );
    }
};
