/**
 * @file        main.cpp
 * @description Draw colored triangle using x11-opengl
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

#include "X11/XKBlib.h"
#include "X11/Xlib.h"
#include "cstdlib"
#include "shader.h"
#include <GL/gl.h>
#include <GL/glew.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <cstddef>
#include <cstdlib>
#include <iostream>

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
    GLuint vertexArrayBuffer     = 0U;
    GLuint indexBuffer           = 0U;
    GLboolean shouldDraw         = false;

    // clang-format off
    static const GLfloat vertexBufferData[] = {
          // positions          // colors           // texture coords
          0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right
          0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
          -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
          -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f  // top left
    };
    static const unsigned int indices[] = {
          0, 1, 3, // first triangle
          1, 2, 3  // second triangle
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

    glGenVertexArrays(1, &vertexArrayBuffer);
    glGenBuffers(1, &vertexBuffer);
    glGenBuffers(1, &indexBuffer);

    glBindVertexArray(vertexArrayBuffer);
    /* initialize vertex buffer */
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBufferData), vertexBufferData, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    /* enable color buffer */
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

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
            if (XK_Escape == sym) { globalAbortFlag = true; }
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

        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program);
        
        /* enable vertex buffer */
        glBindVertexArray(vertexArrayBuffer);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT , 0);
        glXSwapBuffers(dpy, w);
    }

    /* resource cleanup */
    glDeleteVertexArrays(1, &vertexArrayBuffer);
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteBuffers(1, &indexBuffer);
    glDeleteProgram(program);
    glXDestroyContext(dpy, ctxt);
    XFreeColormap(dpy, xattr.colormap);
    XDestroyWindow(dpy, w);
    XCloseDisplay(dpy);
    dpy = nullptr;
    return (0);
}
