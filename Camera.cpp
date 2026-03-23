#include "Camera.h"

#include <cmath>

Camera::Camera(int width, int height, glm::vec3 position)
    : Position(position), width(width), height(height)
{
}

void Camera::Matrix(float FOVdeg, float nearPlane, float farPlane, GLuint shaderProgram, const char* uniform) const
{
    const glm::mat4 view = glm::lookAt(Position, Position + Orientation, Up);
    const glm::mat4 projection = glm::perspective(
        glm::radians(FOVdeg),
        static_cast<float>(width) / static_cast<float>(height),
        nearPlane,
        farPlane
    );
    const glm::mat4 cameraMatrix = projection * view;

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, uniform), 1, GL_FALSE, glm::value_ptr(cameraMatrix));
}

void Camera::Inputs(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        Position += speed * Orientation;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        Position -= speed * Orientation;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        Position -= glm::normalize(glm::cross(Orientation, Up)) * speed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        Position += glm::normalize(glm::cross(Orientation, Up)) * speed;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        Position += speed * Up;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        Position -= speed * Up;
    }

    speed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? 0.4f : 0.1f;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

        if (firstClick)
        {
            glfwSetCursorPos(window, width / 2.0, height / 2.0);
            firstClick = false;
        }

        double mouseX = 0.0;
        double mouseY = 0.0;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        const float rotX = sensitivity * static_cast<float>(mouseY - height / 2.0) / static_cast<float>(height);
        const float rotY = sensitivity * static_cast<float>(mouseX - width / 2.0) / static_cast<float>(width);

        const glm::vec3 right = glm::normalize(glm::cross(Orientation, Up));
        const glm::vec3 rotated = glm::rotate(Orientation, glm::radians(-rotX), right);

        if (std::abs(glm::angle(rotated, Up) - glm::radians(90.0f)) <= glm::radians(85.0f))
        {
            Orientation = rotated;
        }

        Orientation = glm::rotate(Orientation, glm::radians(-rotY), Up);
        glfwSetCursorPos(window, width / 2.0, height / 2.0);
    }
    else
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        firstClick = true;
    }
}
