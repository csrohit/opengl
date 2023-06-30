/**
 * @file    main.cpp
 * @brief   manipulate triangle using matrix operations
 * @author  Rohit Nimkar
 * @date    06/25/2023
 * @version 1.0
 */
#include <iostream>
#include <GL/glew.h>

#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/fwd.hpp>
#include "shader.h"

int main(void)
{
    GLFWwindow* window    = nullptr;
    GLenum rc             = 0;
    GLint n               = 0;
    GLuint vertexBufferId = 0;
    GLuint vertexArray    = 0;
    GLuint vertexBuffer   = 0;
    GLint result          = GL_FALSE;
    GLuint program        = -1;

    // Initialise GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize glfw\n";
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(1024, 768, "Red triangle using Shaders", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize glew\n";
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);

    // Create and compile our GLSL program from the shaders
    result = LoadShaders("vertex.glsl", "fragment.glsl", &program);
    if (GL_TRUE != result)
    {
        std::cerr << "Failed to link program\n";
        return -1;
    }

    // clang-format off
    static const GLfloat vertexBufferData[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
    };
    // clang-format on

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBufferData), vertexBufferData, GL_STATIC_DRAW);

    GLuint MatrixID = glGetUniformLocation(program, "MVP");
    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);

    // Camera matrix
    glm::mat4 View = glm::lookAt(
        glm::vec3(4, 3, 3), // Camera is at (4,3,3), in World Space
        glm::vec3(0, 0, 0),                          // and looks at the origin
        glm::vec3(0, 1, 0)                           // Head is up (set to 0,-1,0 to look upside-down)
    );

    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0., 0.)), glm::vec3(2., 2., 2.));

    /* mode-view-projection matrix */
    glm::mat4 MVP = Projection * View * Model;

    do {
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glVertexAttribPointer(0, 3, /* nElements per vertex */
            GL_FLOAT,               /* type */
            GL_FALSE,               /* OGL should not normalize data, as it is already normalized */
            0,                      /* byte offset between generic vertex attributes */
            (void*)0                /* offset to first vertex */
        );

        // Draw the triangle !
        glDrawArrays(GL_TRIANGLES, 0, 3); // 3 indices starting at 0 -> 1 triangle

        glDisableVertexAttribArray(0);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

    /* release resources */
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteVertexArrays(1, &vertexArray);
    glDeleteProgram(program);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return (0);
}
