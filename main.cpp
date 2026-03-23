#include <iostream>
#include <glad/glad.h>   // OpenGL function loader
#include <GLFW/glfw3.h>  // Window creation and input handling
#include <Windows.h>
#include <mmsystem.h>
#include <thread>
#include <atomic>
#include <chrono>
#include <iomanip>
#include <sstream>
#include "test.h"
#pragma comment(lib, "winmm.lib")


// ===== Vertex Shader =====
const char* vertexShaderSource = R"(#version 330 core
layout (location = 0) in vec2 aPos; // Vertex position coming from attribute location 0

void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0); // Convert 2D vertex to clip-space position
}
)";

// ===== Fragment Shader (Mandelbrot) =====
const char* fragmentShaderSource = R"(#version 330 core
out vec4 FragColor; // Final pixel color

uniform vec2 iResolution; // Screen resolution (width, height)
uniform float iTime;      // Time value for animation

// Mandelbrot iteration function
float mandelbrot(vec2 c)
{
    vec2 z = vec2(0.0); // Start with z = 0
    float n = 0.0;      // Iteration counter

    // Iterate the Mandelbrot formula
    for(int i=0;i<256;i++)
    {
        // Complex number square: z = z² + c
        z = vec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y) + c;

        // Escape condition (|z| > 2)
        if(dot(z,z) > 4.0)
            break;

        n += 1.0;
    }

    return n; // Return number of iterations
}

void main()
{   
    // Convert pixel coordinates to normalized coordinates (0..1)
    vec2 uv = gl_FragCoord.xy / iResolution.xy;

    // Animated zoom factor
    float zoo = 0.62 + 0.38*cos(iTime*0.2);
    zoo = pow(zoo,8.0);

    // Map screen coordinates to complex plane
    vec2 c = vec2(-0.745,0.186) + (uv-0.5)*zoo*3.0;                                                                                                                                                           

    // Compute Mandelbrot iterations
    float m = mandelbrot(c);

    // Color palette using cosine function
    vec3 col = vec3(
    sin(m*0.256),
    cos(m*0.1),
    sin(m*0.2)
    );

    // Inside Mandelbrot set → black
    if(m < 1.0)
        col = vec3(0.0);

    FragColor = vec4(col,1.0); // Output final pixel color
   // PlaySound(TEXT("l.wav"), NULL, SND_FILENAME | SND_ASYNC);
}
)";

static int redisplay_interval = 16; // default ~60 FPS
static std::atomic_bool timer_running(false);
static std::thread timer_thread;

// Helper timer loop using GLFW (without GLUT)
static void timer_loop()
{
    while (timer_running.load())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(redisplay_interval));
        // Wake up the GLFW event loop — the main loop will perform rendering
        glfwPostEmptyEvent();
    }
}

// Compatibility stub (in case some code expects a function with this name)
static void timer(int) { /* not used, kept for compatibility */ }

static void setFPS(int fps) {
    if (fps <= 0) fps = 1;
    redisplay_interval = 1000 / fps;
    if (timer_running.load()) return;
    timer_running = true;
    timer_thread = std::thread(timer_loop);
    timer_thread.detach();
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
int main()
{
    glfwInit();
    // Specify OpenGL version (3.3 Core)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 800, "Mandelbrot", NULL, NULL);

    if (!window)
    {
        std::cout << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window); // Make OpenGL context current
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // Load OpenGL function pointers using GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to init GLAD\n";
        return -1;
    }

    glViewport(0, 0, 800, 800); // Define rendering viewport

    // ===== FULLSCREEN QUAD =====
    // Two triangles forming a rectangle that covers the entire screen
    float vertices[] =
    {
        -1.0f,-1.0f,
         1.0f,-1.0f,
         1.0f, 1.0f,

        -1.0f,-1.0f,
         1.0f, 1.0f,
        -1.0f, 1.0f
    };

    GLuint VAO, VBO;

    glGenVertexArrays(1, &VAO); // Create Vertex Array Object
    glGenBuffers(1, &VBO);      // Create Vertex Buffer Object

    glBindVertexArray(VAO);     // Bind VAO

    glBindBuffer(GL_ARRAY_BUFFER, VBO); // Bind VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Upload vertex data to GPU

    // Describe vertex attribute layout
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0); // Enable vertex attribute

    // ===== SHADERS =====
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER); // Create vertex shader
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL); // Attach shader source
    glCompileShader(vertexShader); // Compile vertex shader

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // Create fragment shader
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL); // Attach shader source
    glCompileShader(fragmentShader); // Compile fragment shader

    GLuint shaderProgram = glCreateProgram(); // Create shader program

    glAttachShader(shaderProgram, vertexShader);   // Attach vertex shader
    glAttachShader(shaderProgram, fragmentShader); // Attach fragment shader

    glLinkProgram(shaderProgram); // Link shaders into program

    glDeleteShader(vertexShader);   // Delete shader objects (not needed after linking)
    glDeleteShader(fragmentShader);

    // ===== MAIN LOOP =====
    // FPS calculation moved inside the main loop (removed unnecessary outer while)
    while (!glfwWindowShouldClose(window)) // while the window should not close
    {
        // FPS calculation and window title update
        static double lastTime = glfwGetTime();
        static int nbFrames = 0;
        double currentTime = glfwGetTime();
        nbFrames++;
        if (currentTime - lastTime >= 1.0) {
            double fps = double(nbFrames) / (currentTime - lastTime);
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2) << fps << " FPS";
            glfwSetWindowTitle(window, ss.str().c_str());
            nbFrames = 0;
            lastTime += 1.0;
        }

        glClear(GL_COLOR_BUFFER_BIT); // Clear the screen

        glUseProgram(shaderProgram); // Use shader program

        double time = glfwGetTime(); // Get current time (glfw returns double)

        // Pass uniforms to shader
        // Pass uniforms to shader
        glUniform1f(glGetUniformLocation(shaderProgram, "iTime"), static_cast<float>(time));

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        glUniform2f(
            glGetUniformLocation(shaderProgram, "iResolution"),
            (float)width,
            (float)height
        );

        glBindVertexArray(VAO); // Bind geometry
        glDrawArrays(GL_TRIANGLES, 0, 6); // Draw fullscreen quad

        glfwSwapBuffers(window); // Swap front/back buffers
        glfwPollEvents(); // Process input events
    }

    // Cleanup GPU resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwDestroyWindow(window); // Destroy window
    glfwTerminate(); // Terminate GLFW

    return 0;
}