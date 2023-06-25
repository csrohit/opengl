/**
 * @file    main.cpp
 * @brief   Print OpenGL extensions
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
    GLFWwindow* window;
    GLenum rc;
    GLint n  = 0; 

    glewExperimental = true;    /* needed for core profile */ 
    /* Initialize the library */
    if (!glfwInit())
    {
        std::cerr<< "failed to initialize glfw \n";
        return -1;
    }

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(800, 600, "My First window on GLFW for mac", NULL, NULL);
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

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    do
    {
        static GLint counter = 0;
        counter++;
        std::cout << "Inside loop: " << counter << std::endl;
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwPollEvents();

    }while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0);

    std::cout << "Author: Rohit Nimkar\n";
    glfwTerminate();
    return 0;
}

