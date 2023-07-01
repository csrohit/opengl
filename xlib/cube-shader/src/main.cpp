/**
 * @file        main.cpp
 * @description Draw colored cube using x11-opengl
 * @author      Rohit Nimkar
 * @version     1.0
 * @date        2023-07-01
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

#define GLX_MAJOR_MIN 1
#define GLX_MINOR_MIN 2

int main()
{
    Display* dpy                 = nullptr;
    Window root                  = 0UL;
    Window w                     = 0UL;
    int scr                      = 0;
    GLint glxMajor               = 0;
    GLint glxMinor               = 0;
    XVisualInfo* vi              = nullptr;
    XSetWindowAttributes xattr   = {};
    bool globalAbortFlag         = false;
    static Atom wm_delete_window = 0;
    GLXContext ctxt              = nullptr;
    GLint result                 = 0;
    GLuint program               = 0U;
    GLuint vertexBuffer          = 0U;
    GLuint colorBuffer           = 0U;
    GLboolean shouldDraw         = false;

    // clang-format off
    static const GLfloat vertexBufferData[] = {
        -1.0f,-1.0f,-1.0f, // triangle 1 : begin
        -1.0f,-1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f, // triangle 1 : end
        1.0f, 1.0f,-1.0f, // triangle 2 : begin
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f, // triangle 2 : end
        1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
        1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        1.0f,-1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f,-1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f,-1.0f, 1.0f
    };
    static const GLfloat colorBufferData[] = {
        0.583f,  0.771f,  0.014f,
        0.609f,  0.115f,  0.436f,
        0.327f,  0.483f,  0.844f,
        0.822f,  0.569f,  0.201f,
        0.435f,  0.602f,  0.223f,
        0.310f,  0.747f,  0.185f,
        0.597f,  0.770f,  0.761f,
        0.559f,  0.436f,  0.730f,
        0.359f,  0.583f,  0.152f,
        0.483f,  0.596f,  0.789f,
        0.559f,  0.861f,  0.639f,
        0.195f,  0.548f,  0.859f,
        0.014f,  0.184f,  0.576f,
        0.771f,  0.328f,  0.970f,
        0.406f,  0.615f,  0.116f,
        0.676f,  0.977f,  0.133f,
        0.971f,  0.572f,  0.833f,
        0.140f,  0.616f,  0.489f,
        0.997f,  0.513f,  0.064f,
        0.945f,  0.719f,  0.592f,
        0.543f,  0.021f,  0.978f,
        0.279f,  0.317f,  0.505f,
        0.167f,  0.620f,  0.077f,
        0.347f,  0.857f,  0.137f,
        0.055f,  0.953f,  0.042f,
        0.714f,  0.505f,  0.345f,
        0.783f,  0.290f,  0.734f,
        0.722f,  0.645f,  0.174f,
        0.302f,  0.455f,  0.848f,
        0.225f,  0.587f,  0.040f,
        0.517f,  0.713f,  0.338f,
        0.053f,  0.959f,  0.120f,
        0.393f,  0.621f,  0.362f,
        0.673f,  0.211f,  0.457f,
        0.820f,  0.883f,  0.371f,
        0.982f,  0.099f,  0.879f
    };
    // clang-format on

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
    xattr.event_mask        = ExposureMask | KeyPressMask | StructureNotifyMask;

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
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBufferData), vertexBufferData, GL_STATIC_DRAW);

    /* initialize color buffer */
    glGenBuffers(1, &colorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colorBufferData), colorBufferData, GL_STATIC_DRAW);

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
    GLuint MatrixID      = glGetUniformLocation(program, "MVP");
    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
    glm::mat4 View       = glm::lookAt(glm::vec3(4, 3, -3), // Camera is at (4,3,3), in World Space
              glm::vec3(0, 0, 0),                           // and looks at the origin
              glm::vec3(0, 1, 0)                            // Head is up (set to 0,-1,0 to look upside-down)
          );
    glm::mat4 Model      = glm::mat4(1.0f);
    glm::mat4 MVP        = Projection * View * Model;

    while (!globalAbortFlag)
    {
        XEvent evt;
        XNextEvent(dpy, &evt);
        switch (evt.type)
        {
        case Expose:
        {
            if (!shouldDraw) shouldDraw = true;
            break;
        }
        case ClientMessage:
        {
            if (evt.xclient.data.l[0] == wm_delete_window) { globalAbortFlag = true; }
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
            std::cout << "Default event: " << evt.type << std::endl;
            break;
        }
        }

        if (!shouldDraw) continue;

        /* redraw frame */
        std::cout << "redrawing frame" << std::endl;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        /* enable vertex buffer */
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        /* enable color buffer */
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glDrawArrays(GL_TRIANGLES, 0, 36);
        glXSwapBuffers(dpy, w);
    }

    /* resource cleanup */
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteProgram(program);
    glXDestroyContext(dpy, ctxt);
    XFreeColormap(dpy, xattr.colormap);
    XDestroyWindow(dpy, w);
    XCloseDisplay(dpy);
    dpy = nullptr;
    return (0);
}
