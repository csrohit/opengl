/**
 * @file        main.cpp
 * @description Load cube model using VAO
 * @author      Rohit Nimkar
 * @version     1.0
 * @date        2023-11-01
 * @copyright   Copyright 2023 Rohit Nimkar
 *
 * @attention
 *  Use of this source code is governed by a BSD-style
 *  license that can be found in the LICENSE file or at
 *  opensource.org/licenses/BSD-3-Clause
 */

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "X11/Xlib.h"
#include "cstdlib"
#include <GL/glx.h>
#include <GL/gl.h>
#include <X11/keysymdef.h>
#include "X11/XKBlib.h"
#include "shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include "vmath.h"
#include "stb_image.h"

#define GLX_MAJOR_MIN 1
#define GLX_MINOR_MIN 2

/**
 * @class Header
 * @brief Header of the Mode hex file
 *
 */
struct Header
{
    /**
     * @brief total number of vertices in the model
     */
    int nVertices;

    /**
     * @brief total number of indices in the model
     */
    int nIndices;
};

/**
 * @class Vertex
 * @brief Describes the properties of a vertex loaded from model
 */
struct Vertex
{
    /**
     * @brief X Co-ordinate of position
     */
    float x;
    /**
     * @brief Y Co-ordinate of position
     */
    float y;
    /**
     * @brief Z Co-ordinate of position
     */
    float z;
    /**
     * @brief U Co-ordinate of texture
     */
    float u;
    /**
     * @brief V Co-ordinate of texture
     */
    float v;
};

int main()
{
    /* Windowing related variables */
    Display*             dpy              = nullptr; // connection to server
    Window               root             = 0UL;     // handle of root window [Desktop]
    Window               w                = 0UL;     // handle of current window
    int                  scr              = 0;       // handle to DefaultScreen
    XVisualInfo*         vi               = nullptr; // Pointer to current visual info
    static Atom          wm_delete_window = 0;       // atomic variable to detect close button click
    XSetWindowAttributes xattr            = {};      // structure for windows attributes
    bool                 globalAbortFlag  = false;

    /* OpenGl related variables */
    GLint      glxMinor = 0;       // major version of glx library
    GLint      glxMajor = 0;       // minor version of glx library
    GLXContext ctxt     = nullptr; // handle to OpenGL context

    /* Variables related to current program */
    GLint     result       = 0;     // variable to get value returned by APIS
    GLuint    program      = 0U;    // handle of shader program
    GLuint    vertexBuffer = 0U;    // handle of vertex buffer
    GLuint    texture      = 0U;    // handle to texture
    GLboolean shouldDraw   = false; // decide to render or not

    /* Variables related to texture */
    GLint width      = 0; // width of texture
    GLint height     = 0; // height of texture
    GLint nrChannels = 0; // number of color channels in image

    struct Vertex* vertices = NULL;
    struct Header  header;
    FILE*          pFile = fopen("./model.hex", "rb");
    if (NULL == pFile)
    {
        printf("Failed to read file\n");
        return EXIT_FAILURE;
    }

    fread(&header, sizeof(header), 1, pFile);
    printf("number of vertices: %d\n", header.nVertices);
    vertices = (struct Vertex*)malloc(sizeof(struct Vertex) * header.nVertices);
    fread(vertices, sizeof(struct Vertex), header.nVertices, pFile);

    for (uint32_t idx = 0; idx < header.nVertices; ++idx) { printf("[%f %f %f : %f %f]\n", vertices[idx].x, vertices[idx].y, vertices[idx].z, vertices[idx].u, vertices[idx].v); }

    fclose(pFile);

    dpy = XOpenDisplay(NULL);
    if (!glXQueryVersion(dpy, &glxMajor, &glxMinor))
    {
        std::cerr << "Failed to query glx version\n";
        exit(EXIT_FAILURE);
    }
    if (glxMajor < GLX_MAJOR_MIN && glxMinor < GLX_MINOR_MIN)
    {
        std::cerr << "GLX version >=1.2 is required\n";
        exit(EXIT_FAILURE);
    }
    std::cout << "glx version is " << glxMajor << "." << glxMinor << std::endl;

    scr  = DefaultScreen(dpy);
    root = XDefaultRootWindow(dpy);

    // clang-format off
    GLint glxAttriutes[] = {
        GLX_RGBA,
        GLX_DOUBLEBUFFER,
        GLX_DEPTH_SIZE, 24,
        GLX_STENCIL_SIZE, 8,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_SAMPLE_BUFFERS, 0,
        GLX_SAMPLES, 0,
        None
    };
    // clang-format on

    vi = glXChooseVisual(dpy, scr, glxAttriutes);
    if (nullptr == vi)
    {
        std::cerr << "Could not create required visual window\n";
        exit(EXIT_FAILURE);
    }

    /* set window attributes */
    xattr.border_pixel      = BlackPixel(dpy, scr);
    xattr.background_pixel  = WhitePixel(dpy, scr);
    xattr.override_redirect = true;
    xattr.colormap          = XCreateColormap(dpy, root, vi->visual, AllocNone);
    xattr.event_mask        = ExposureMask | KeyPressMask | StructureNotifyMask | PointerMotionMask;

    /* create window */
    w = XCreateWindow(dpy, root, 0, 0, 1024, 768, 0, vi->depth, InputOutput, vi->visual, CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, &xattr);
    XStoreName(dpy, w, "Rohit Nimkar: OpenGL demo with X11");

    /* create opengl context */
    ctxt = glXCreateContext(dpy, vi, nullptr, GL_TRUE);
    glXMakeCurrent(dpy, w, ctxt);

    /* initialize glew */
    glewExperimental = true;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize glew\n";
        XFree(vi);
        XFreeColormap(dpy, xattr.colormap);
        glXDestroyContext(dpy, ctxt);
        XDestroyWindow(dpy, w);
        XCloseDisplay(dpy);
        return -1;
    }

    /* free XVisual as it is not required */
    XFree(vi);
    vi = nullptr;

    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    /* initialize vertex buffer */
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(struct Vertex) * header.nVertices, vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)0);

    /* load and create texture */
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)(3 * sizeof(GLfloat)));


    /* set the texture wrapping parameters */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    /* set texture filtering parameters */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /* load image, create texture and generate mipmaps */
    unsigned char* data = stbi_load("./wall.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    // Create and compile our GLSL program from the shaders
    result = LoadShaders("vertex.glsl", "fragment.glsl", &program);

    if (GL_TRUE != result)
    {
        std::cerr << "Failed to link program\n";
        return -1;
    }

    /* register for window close event */
    wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpy, w, &wm_delete_window, 1);
    XMapWindow(dpy, w);

    /* generate transformation matrix */
    GLuint      MatrixID   = glGetUniformLocation(program, "MVP");
    vmath::mat4 Projection = vmath::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    vmath::mat4 View       = vmath::lookat(vmath::vec3(1.0f, 3.0f, 5.0f), vmath::vec3(0.0f, 0.0f, 0.0f), vmath::vec3(0.0f, 1.0f, 0.0f));
    vmath::mat4 Model      = vmath::mat4::identity();
    vmath::mat4 MVP        = Projection * View * Model;
    while (!globalAbortFlag)
    {
        XEvent evt;
        if (XPending(dpy))
        {
            XNextEvent(dpy, &evt);
            switch (evt.type)
            {
                case Expose:
                {
                    if (!shouldDraw)
                        shouldDraw = true;
                    break;
                }
                case ClientMessage:
                {
                    if (evt.xclient.data.l[0] == wm_delete_window)
                    {
                        globalAbortFlag = true;
                    }
                    break;
                }
                case KeyPress:
                {
                    KeySym sym = XkbKeycodeToKeysym(dpy, evt.xkey.keycode, 0, 0);
                    if (XK_Escape == sym)
                    {
                        globalAbortFlag = true;
                        shouldDraw      = false;
                    }
                    break;
                }
                case MapNotify:
                {
                    std::cout << "GL Vendor: " << glGetString(GL_VENDOR) << "\n";
                    std::cout << "GL Renderer: " << glGetString(GL_RENDERER) << "\n";
                    std::cout << "GL Version: " << glGetString(GL_VERSION) << "\n";
                    std::cout << "GL Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
                    break;
                }
                default:
                {
                    // std::cout << "Default event: " << evt.type << std::endl;
                    break;
                }
            }
        }

        if (!shouldDraw)
            continue;

        /* redraw frame */
        // std::cout << "redrawing frame" << std::endl;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        static GLfloat theta = 3;
        theta += 0.5;
        Model = vmath::translate(0.0f, 0.0f, 0.0f) * vmath::rotate(theta, 0.0f, 1.0f, 0.0f) * vmath::scale(1.0f, 1.0f, 1.0f);
        MVP   = Projection * View * Model;

        glUseProgram(program);
        glEnableVertexAttribArray(1);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

        /* enable vertex buffer */
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

        glDrawArrays(GL_TRIANGLES, 0, header.nVertices);
        glXSwapBuffers(dpy, w);
    }

    free(vertices);
    /* resource cleanup */
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteTextures(1, &texture);
    glDeleteProgram(program);
    glXDestroyContext(dpy, ctxt);
    XFreeColormap(dpy, xattr.colormap);
    XDestroyWindow(dpy, w);
    XCloseDisplay(dpy);
    dpy = nullptr;
    return (0);
}
