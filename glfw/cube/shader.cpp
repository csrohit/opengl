#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include "shader.h"

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path, GLuint* pProgram)
{
    // Create the shaders
    GLuint vertexShader   = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    GLint result;

    result = loadShader(vertexShader, vertex_file_path);
    if (GL_TRUE != result) { return (result); }
    result = loadShader(fragmentShader, fragment_file_path);
    if (GL_TRUE != result)
    {
        std::cout << "Deleting vertex shader\n";
        glDeleteShader(vertexShader);
        std::cout << "Deleting vertex shader\n";
        return (result);
    }

    // Link the program
    std::cout << "Linking program\n";
    *pProgram = glCreateProgram();
    glAttachShader(*pProgram, vertexShader);
    glAttachShader(*pProgram, fragmentShader);
    glLinkProgram(*pProgram);

    // Check the program
    glGetProgramiv(*pProgram, GL_LINK_STATUS, &result);
    if (GL_TRUE != result)
    {
        // linking Failed
        int infoLogLen = 0;
        std::cerr << "Failed to link program\nwooreogieogieroigeorni";
        glGetProgramiv(*pProgram, GL_INFO_LOG_LENGTH, &infoLogLen);
        if (infoLogLen > 0)
        {
            char msg[infoLogLen + 1];
            glGetProgramInfoLog(*pProgram, infoLogLen, nullptr, msg);
            std::cerr << msg << std::endl;
        }
        return (result);
    }

    glDetachShader(*pProgram, vertexShader);
    glDetachShader(*pProgram, fragmentShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return (result);
}

GLint loadShader(GLuint shaderId, const char* pFilename)
{
    GLint result = GL_FALSE;
    std::string sourceString;
    std::ifstream sourceInputStream(pFilename, std::ios::in);

    if (sourceInputStream.is_open())
    {
        std::stringstream sstr;
        sstr << sourceInputStream.rdbuf();
        sourceString = sstr.str();
        sourceInputStream.close();
    }
    else
    {
        std::cerr << "failed to read shader" << pFilename << std::endl;
        return (GL_FALSE);
    }

    // Compile shader
    std::cout << "Compiling shader: " << pFilename << std::endl;
    char const* pSourceCode = sourceString.c_str();
    glShaderSource(shaderId, 1, &pSourceCode, NULL);
    glCompileShader(shaderId);

    // validate compilation status
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
    if (GL_TRUE != result)
    {
        int infoLogLen = 0;
        std::cerr << "Failed to compile shader: " << pFilename << std::endl;

        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLen);
        if (infoLogLen > 0)
        {
            char msg[infoLogLen];
            glGetShaderInfoLog(shaderId, infoLogLen, NULL, msg);
            std::cerr << msg << std::endl;
        }
    }
    return (result);
}
