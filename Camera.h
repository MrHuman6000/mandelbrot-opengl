#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

class Camera
{
public:
    glm::vec3 Position;
    glm::vec3 Orientation{ 0.0f, 0.0f, -1.0f };
    glm::vec3 Up{ 0.0f, 1.0f, 0.0f };

    int width;
    int height;

    float speed = 0.1f;
    float sensitivity = 100.0f;
    bool firstClick = true;

    Camera(int width, int height, glm::vec3 position);

    void Matrix(float FOVdeg, float nearPlane, float farPlane, GLuint shaderProgram, const char* uniform) const;
    void Inputs(GLFWwindow* window);
};
