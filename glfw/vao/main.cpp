/**
 * @file    main.cpp
 * @brief   Draw first triangle
 * @author  Rohit Nimkar
 * @date    06/25/2023
 * @version 1.0
 */
#include <cstdio>
#include <iostream>
#include <GL/glew.h>

#if defined(__APPLE__)
    #include <OpenGL/gl.h>
#else 
    #include <GL/gl.h>
#endif

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <ostream>


int main(void)
{
    GLFWwindow* window    = nullptr;
    GLenum rc             = 0;
    GLint  n              = 0;
    GLuint vertexBufferId = 0;


    glewExperimental = true;    /* needed for core profile */ 
    /* Initialize the library */
    if (!glfwInit())
    {
        std::cerr<< "failed to initialize glfw \n";
        return -1;
    }

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(800, 600, "Whilte triangle using default shader   ", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    rc = glewInit();
    if(GLEW_OK != rc){
        /* glewinit() failed */
        std::cout << "glewInit() failed: " << glewGetErrorString(rc);
    }

    printf("%-20s:%s\n", "GPU Vendor" , glGetString(GL_VENDOR));
    printf("%-20s:%s\n", "Version String" , glGetString(GL_VERSION));
    printf("%-20s:%s\n", "Graphics Renderer" , glGetString(GL_RENDERER));
    printf("%-20s:%s\n", "GL Shading Language" , glGetString(GL_SHADING_LANGUAGE_VERSION));


    static const GLfloat vertex_bufferData[] = 
        {
            -1.0f, -1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
        };

    glGenBuffers(1, &vertexBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_bufferData), vertex_bufferData, GL_STATIC_DRAW);

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    do
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
        glVertexAttribPointer(
           0,                  
           3,                  /* nElements per vertex */
           GL_FLOAT,           /* type */
           GL_FALSE,           /* OGL should not normalize data, as it is already normalized */
           0,                  /* byte offset between generic vertex attributes */
           (void*)0            /* offset to first vertex */
        );

        // Draw the triangle !
        glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
        glDisableVertexAttribArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();

    }while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0);

    std::cout << "Author: Rohit Nimkar\n";
    glfwTerminate();
    return 0;
}

