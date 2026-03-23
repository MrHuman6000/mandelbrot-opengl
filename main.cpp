#include <iostream>
#include <iomanip>
#include <sstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "test.h"

const char* vertexShaderSource = R"(#version 330 core
layout (location = 0) in vec2 aPos;

void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

const char* fragmentShaderSource = R"(#version 330 core
out vec4 FragColor;

uniform vec2 iResolution;
uniform float iTime;

float mandelbrot(vec2 c)
{
    vec2 z = vec2(0.0);
    float n = 0.0;

    for (int i = 0; i < 256; ++i)
    {
        z = vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y) + c;

        if (dot(z, z) > 4.0)
        {
            break;
        }

        n += 1.0;
    }

    return n;
}

void main()
{
    vec2 uv = gl_FragCoord.xy / iResolution.xy;

    float zoo = 0.62 + 0.38 * cos(iTime * 0.2);
    zoo = pow(zoo, 8.0);

    vec2 c = vec2(-0.745, 0.186) + (uv - 0.5) * zoo * 3.0;
    float m = mandelbrot(c);

    vec3 col = vec3(
        sin(m * 0.256),
        cos(m * 0.1),
        sin(m * 0.2)
    );

    if (m < 1.0)
    {
        col = vec3(0.0);
    }

    FragColor = vec4(col, 1.0);
}
)";

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    (void)window;
    glViewport(0, 0, width, height);
}

int main()
{
    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 800, "Mandelbrot", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cout << "Failed to initialize GLAD\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glViewport(0, 0, 800, 800);

    const float vertices[] =
    {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,

        -1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f
    };

    GLuint vao = 0;
    GLuint vbo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    const GLuint shaderProgram = createShader(vertexShaderSource, fragmentShaderSource);
    if (shaderProgram == 0)
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    const GLint timeLocation = glGetUniformLocation(shaderProgram, "iTime");
    const GLint resolutionLocation = glGetUniformLocation(shaderProgram, "iResolution");
    if (timeLocation == -1 || resolutionLocation == -1)
    {
        std::cout << "Failed to get shader uniform locations\n";
        glDeleteProgram(shaderProgram);
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    while (!glfwWindowShouldClose(window))
    {
        static double lastTime = glfwGetTime();
        static int frameCount = 0;

        const double currentTime = glfwGetTime();
        ++frameCount;
        if (currentTime - lastTime >= 1.0)
        {
            const double fps = static_cast<double>(frameCount) / (currentTime - lastTime);
            std::stringstream title;
            title << "Mandelbrot - " << std::fixed << std::setprecision(2) << fps << " FPS";
            glfwSetWindowTitle(window, title.str().c_str());
            frameCount = 0;
            lastTime = currentTime;
        }

        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);

        const float time = static_cast<float>(glfwGetTime());
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(window, &width, &height);

        glUniform1f(timeLocation, time);
        glUniform2f(resolutionLocation, static_cast<float>(width), static_cast<float>(height));

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shaderProgram);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
