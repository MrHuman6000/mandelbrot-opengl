#include <glad/glad.h>

#include <iostream>
#include <string>

#include "test.h"

namespace
{
    bool checkShader(GLuint shader, const char* stageName)
    {
        GLint success = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (success == GL_TRUE)
        {
            return true;
        }

        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::string log(static_cast<size_t>(logLength > 1 ? logLength - 1 : 0), '\0');
        if (logLength > 1)
        {
            glGetShaderInfoLog(shader, logLength, nullptr, log.data());
        }

        std::cerr << stageName << " shader compilation failed";
        if (!log.empty())
        {
            std::cerr << ":\n" << log;
        }
        std::cerr << '\n';
        return false;
    }

    bool checkProgram(GLuint program)
    {
        GLint success = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (success == GL_TRUE)
        {
            return true;
        }

        GLint logLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        std::string log(static_cast<size_t>(logLength > 1 ? logLength - 1 : 0), '\0');
        if (logLength > 1)
        {
            glGetProgramInfoLog(program, logLength, nullptr, log.data());
        }

        std::cerr << "Shader program link failed";
        if (!log.empty())
        {
            std::cerr << ":\n" << log;
        }
        std::cerr << '\n';
        return false;
    }
}

unsigned int createShader(const char* vertexSrc, const char* fragmentSrc)
{
    const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSrc, nullptr);
    glCompileShader(vertexShader);
    if (!checkShader(vertexShader, "Vertex"))
    {
        glDeleteShader(vertexShader);
        return 0;
    }

    const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSrc, nullptr);
    glCompileShader(fragmentShader);
    if (!checkShader(fragmentShader, "Fragment"))
    {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }

    const GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    if (!checkProgram(program))
    {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(program);
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}
